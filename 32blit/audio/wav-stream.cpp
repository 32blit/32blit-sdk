#include <cinttypes>
#include <cstring>

#include "wav-stream.hpp"

#include "audio/audio.hpp"
#include "engine/engine.hpp"
#include "engine/file.hpp"

namespace blit {

  struct WAVHeader {
    char riff_id[4];
    uint32_t riff_size;
    char riff_format[4];

    char fmt_id[4];
    uint32_t fmt_size;
    uint16_t fmt_format;
    uint16_t fmt_channels;
    uint32_t fmt_sample_rate;
    uint32_t fmt_byte_rate;
    uint16_t fmt_block_align;
    uint16_t fmt_bits_per_sample;

    char data_id[4];
    uint32_t data_size;
  };

  WavStream::WavStream() {
  }

  WavStream::~WavStream() {
    if(!file.get_ptr()) {
      delete[] file_buffers[0];
      delete[] file_buffers[1];
    }
  }

  bool WavStream::load(std::string filename) {
    if(channel != -1)
      blit::channels[channel].off();

    current_sample = nullptr;
    buffered_samples = 0;

    // avoid attempting to free later
    if(file.get_ptr())
      file_buffers[0] = file_buffers[1] = nullptr;

    if(!file.open(filename))
      return false;

    // don't need a buffer if it's in flash
    if(file.get_ptr()) {
      delete[] file_buffers[0];
      delete[] file_buffers[1];
      file_buffers[0] = file_buffers[1] = nullptr;
    }

    // fill initial buffer
    file_buffer_filled[0] = file_buffer_filled[1] = 0;
    file_offset = 0;
    file_data_len = 0;
    read(0);

    if(!file_buffer_filled[0])
      return false;

    // check header
    auto head = reinterpret_cast<const WAVHeader *>(file_buffers[0]);

    // some validation
    if(memcmp(head->riff_id, "RIFF", 4) != 0 || memcmp(head->riff_format, "WAVE", 4) != 0 ||
      memcmp(head->fmt_id, "fmt ", 4) != 0 || memcmp(head->data_id, "data", 4) != 0) {
      return false;
    }

    // some restrictions
    // just refusing to play anything that wastes space
    if(head->fmt_format != 1 /*PCM*/ || head->fmt_channels != 1 || (head->fmt_sample_rate != 22050 && head->fmt_sample_rate != 11025))
      return false;

    bits_per_sample = head->fmt_bits_per_sample;
    sample_rate = head->fmt_sample_rate;
    file_data_len = head->data_size;

    // advance buf past header
    if(file.get_ptr()) {
      // in-memory, adjust the ptr
      file_buffers[0] += sizeof(WAVHeader);
      file_buffer_filled[0] = file_data_len;
    } else {
      file_buffer_filled[0] -= sizeof(WAVHeader);
      memmove(file_buffers[0], file_buffers[0] + sizeof(WAVHeader), file_buffer_filled[0]);
    }

    duration_ms = uint64_t(file_data_len) * 1000 / (sample_rate * bits_per_sample / 8);

    // fill second buffer too
    read(1);

    return true;
  }

  void WavStream::play(int channel, int flags) {
    if(!file_buffer_filled[0])
      return;

    pause();

    this->channel = channel;
    this->play_flags = flags;

    if((flags & PlayFlags::from_start) && buffered_samples)
      restart();

    if(!current_sample) {
      cur_play_buf = 0;
      current_sample = file_buffers[0];
      data_end = file_buffers[0] + file_buffer_filled[0];
      blit::channels[channel].wave_buf_pos = 0;
    }

    blit::channels[channel].waveforms = blit::Waveform::WAVE;
    blit::channels[channel].user_data = this;
    blit::channels[channel].wave_buffer_callback = &WavStream::static_callback;

    blit::channels[channel].trigger_sustain();
  }

  void WavStream::pause() {
    if(channel != -1)
      blit::channels[channel].off();
  }

  void WavStream::restart() {
    bool was_playing = get_playing();
    pause();

    // reset file buffer
    file_buffer_filled[0] = file_buffer_filled[1] = 0;
    file_offset = 0;
    read(0);

    // reset sample buffer
    current_sample = nullptr;
    buffered_samples = 0;


    if(was_playing)
      play(channel, play_flags);
  }

  bool WavStream::get_playing() const {
    return channel != -1 && blit::channels[channel].adsr_phase == blit::ADSRPhase::SUSTAIN;
  }

  int WavStream::get_play_flags() const {
    return play_flags;
  }

  void WavStream::update() {
    if(!current_sample)
      return;

    // refill file buffers
    for(int i = 0; i < 2; i++) {
      if(!file_buffer_filled[i])
        read(i);
    }
  }

  unsigned int WavStream::get_current_sample() const {
    return buffered_samples + blit::channels[channel].wave_buf_pos;
  }

  int WavStream::get_duration_ms() const {
    return duration_ms;
  }

  void WavStream::static_callback(AudioChannel &channel) {
    reinterpret_cast<WavStream *>(channel.user_data)->callback(channel);
  }

  void WavStream::callback(AudioChannel &channel) {
    if(!current_sample) {
      channel.off();
      return;
    }

    // there was no buffer last time
    if(current_sample == data_end) {
      if(file_buffer_filled[cur_play_buf] == -1) {
        current_sample = data_end = nullptr;
        channel.off();
        return;
      } else if(file_buffer_filled[cur_play_buf]) {
        data_end = file_buffers[cur_play_buf] + file_buffer_filled[cur_play_buf]; // recovered from underrun
      } else {
        memset(channel.wave_buffer, 0, 64 * sizeof(int16_t));
        return;
      }
    }

    auto out = channel.wave_buffer;

    int i = 0;
    for(; i < 64 && current_sample; i++) {

      if(bits_per_sample == 8) // 8-bit
        *(out++) = (*current_sample << 8)  - 0x7F00;
      else // 16-bit
        *(out++) = *(const int16_t *)current_sample;

      // the other rate is half, so only increment on even samples
      if(sample_rate == 22050 || i & 1)
        current_sample += bits_per_sample / 8;

      // swap buffers
      if(current_sample == data_end) {
        file_buffer_filled[cur_play_buf] = 0;
        cur_play_buf++;
        cur_play_buf %= 2;

        if(file_buffer_filled[cur_play_buf] == -1) // EOF
          current_sample = data_end = nullptr;
        else {
          current_sample = file_buffers[cur_play_buf];
          data_end = current_sample + file_buffer_filled[cur_play_buf];
          // if(current_sample == end_sample) underrun
        }
      }
    }

    buffered_samples += i;

    for(; i < 64; i++)
      (*out++) = 0;
  }

  void WavStream::read(int buffer) {
    // init buffer
    if(file.get_ptr()) {
      // we have the whole thing
      if(buffer == 0 || (play_flags & PlayFlags::loop)) {
        file_buffers[buffer] = const_cast<uint8_t *>(file.get_ptr());
        file_buffer_filled[buffer] = file.get_length();

        if(file_data_len) {
          // skip header and set correct data size for loop
          file_buffers[buffer] += sizeof(WAVHeader);
          file_buffer_filled[buffer] = file_data_len;
        }
      }
      return;
    } else if(!file_buffers[buffer]) {
      file_buffers[buffer] = new uint8_t[file_buffer_size];
    }

    // don't read past end of data chunk
    uint32_t max_len = file_buffer_size;
    if(file_data_len && file_data_len - file_offset < max_len)
      max_len = file_data_len - file_offset;

    auto read = file.read(file_offset, max_len, reinterpret_cast<char *>(file_buffers[buffer]));

    if(read <= 0) {
      if(play_flags & PlayFlags::loop) {
        // retry at start
        file_offset = sizeof(WAVHeader);
        read = file.read(file_offset, file_buffer_size, reinterpret_cast<char *>(file_buffers[buffer]));
      } else {
        // else mark EOF
        file_buffer_filled[buffer] = -1;
        return;
      }
    }

    file_buffer_filled[buffer] = read;
    file_offset += read;
  }
}

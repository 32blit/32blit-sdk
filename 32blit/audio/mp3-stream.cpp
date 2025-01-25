#include <cinttypes>
#include <cstring>

#include "mp3-stream.hpp"

#define MINIMP3_IMPLEMENTATION
#define MINIMP3_ONLY_MP3
#include "minimp3.h"

#include "audio/audio.hpp"
#include "engine/engine.hpp"
#include "engine/file.hpp"

namespace blit {
  MP3Stream::MP3Stream() {
    mp3dec = new mp3dec_t;
  }

  MP3Stream::~MP3Stream() {
    delete static_cast<mp3dec_t *>(mp3dec);
  }

  bool MP3Stream::load(std::string filename, bool do_duration_calc) {
    if(channel != -1)
      blit::channels[channel].off();

    current_sample = nullptr;
    buffered_samples = 0;
    need_convert = false;

    // avoid attempting to free later
    if(file.get_ptr())
      file_buffer = nullptr;

    if(!file.open(filename))
      return false;

    // don't need a buffer if it's in flash
    if(file.get_ptr()) {
      delete[] file_buffer;
      file_buffer = nullptr;
    }

    // fill initial buffer
    file_buffer_filled = 0;
    file_offset = 0;
    read(0);

    if(!file_buffer_filled)
      return false;

    if(do_duration_calc) {
      mp3dec_init(static_cast<mp3dec_t *>(mp3dec));
      duration_ms = calc_duration();
    } else
      duration_ms = 0;

    // start the decoder
    mp3dec_init(static_cast<mp3dec_t *>(mp3dec));

    return true;
  }

  void MP3Stream::play(int channel, int flags) {
    if(!file_buffer_filled)
      return;

    pause();

    this->channel = channel;
    this->play_flags = flags;

    if((flags & PlayFlags::from_start) && buffered_samples)
      restart();

    if(!current_sample) {
      decode(0);
      decode(1);

      cur_audio_buf = 0;
      current_sample = audio_buf[0];
      end_sample = current_sample + data_size[0];
      blit::channels[channel].wave_buf_pos = 0;
    }

    blit::channels[channel].waveforms = blit::Waveform::WAVE;
    blit::channels[channel].user_data = this;
    blit::channels[channel].wave_buffer_callback = &MP3Stream::static_callback;

    blit::channels[channel].trigger_sustain();
  }

  void MP3Stream::pause() {
    if(channel != -1)
      blit::channels[channel].off();
  }

  void MP3Stream::restart() {
    bool was_playing = get_playing();
    pause();

    // reset file buffer
    file_buffer_filled = 0;
    file_offset = 0;
    read(0);

    // reset sample buffer
    current_sample = nullptr;
    buffered_samples = 0;

    // re-init decoder
    mp3dec_init(static_cast<mp3dec_t *>(mp3dec));

    if(was_playing)
      play(channel, play_flags);
  }

  bool MP3Stream::get_playing() const {
    return channel != -1 && blit::channels[channel].adsr_phase == blit::ADSRPhase::SUSTAIN;
  }

  int MP3Stream::get_play_flags() const {
    return play_flags;
  }

  void MP3Stream::update() {
    if(!current_sample)
      return;

    // refill audio buffers
    for(int i = 0; i < 2; i++) {
      if(!data_size[i])
        decode(i);
    }
  }

  unsigned int MP3Stream::get_current_sample() const {
    return buffered_samples + blit::channels[channel].wave_buf_pos;
  }

  int MP3Stream::get_duration_ms() const {
    return duration_ms;
  }

  void MP3Stream::decode(int buf_index) {
    mp3dec_frame_info_t info = {};

    int samples = 0;
    int freq_scale = 1;

    while(true) {
      if(file_buffer_filled == 0)
        break;

      if(need_convert) {
        // attempt to convert to mono 22050Hz (badly)
        int16_t tmp_buf[MINIMP3_MAX_SAMPLES_PER_FRAME];
        int tmp_samples = mp3dec_decode_frame(static_cast<mp3dec_t *>(mp3dec), file_buffer, file_buffer_filled, nullptr, &info);

        if(tmp_samples) {
          freq_scale = info.hz / 22050;
          int div = info.channels * freq_scale;

          if(samples + tmp_samples / freq_scale > audio_buf_size)
            break;

          mp3dec_decode_frame(static_cast<mp3dec_t *>(mp3dec), file_buffer, file_buffer_filled, tmp_buf, &info);

          for(int i = 0; i < tmp_samples * info.channels; i += div, samples++) {
            int32_t tmp = 0;
            for(int j = 0; j < div; j++)
              tmp += tmp_buf[i + j];

            audio_buf[buf_index][samples] = tmp / div;
          }
        }
      } else {
        int new_samples = mp3dec_decode_frame(static_cast<mp3dec_t *>(mp3dec), file_buffer, file_buffer_filled, nullptr, &info);
        if(samples + new_samples > audio_buf_size)
          break;

        samples += mp3dec_decode_frame(static_cast<mp3dec_t *>(mp3dec), file_buffer, file_buffer_filled, audio_buf[buf_index] + samples, &info);
      }

      // switch conversion on and retry if needed
      if(!need_convert && (info.channels != 1 || info.hz != 22050)) {
        need_convert = true;
        decode(buf_index);
        return;
      }

      read(info.frame_bytes);
    }

    if(!samples) {
      if(play_flags & PlayFlags::loop) {
        // back to start
        file_buffer_filled = 0;
        file_offset = 0;
        read(0);

        mp3dec_init(static_cast<mp3dec_t *>(mp3dec));
        decode(buf_index);
      } else
        data_size[buf_index] = -1;

      return;
    }

    data_size[buf_index] = samples;
  }

  void MP3Stream::static_callback(AudioChannel &channel) {
    reinterpret_cast<MP3Stream *>(channel.user_data)->callback(channel);
  }

  void MP3Stream::callback(AudioChannel &channel) {
    if(!current_sample) {
      channel.off();
      return;
    }

    // there was no buffer last time
    if(current_sample == end_sample) {
      if(data_size[cur_audio_buf] == -1) {
        current_sample = end_sample = nullptr;
        channel.off();
        return;
      } else if(data_size[cur_audio_buf]) {
        end_sample = audio_buf[cur_audio_buf] + data_size[cur_audio_buf]; // recovered from underrun
      } else {
        memset(channel.wave_buffer, 0, 64 * sizeof(int16_t));
        return;
      }
    }

    auto out = channel.wave_buffer;

    int i = 0;
    for(; i < 64; i++)
      *(out++) = *(current_sample++);


    // swap buffers
    if(current_sample == end_sample) {
      data_size[cur_audio_buf] = 0;
      cur_audio_buf++;
      cur_audio_buf %= 2;

      if(data_size[cur_audio_buf] == -1) // EOF
        current_sample = end_sample = nullptr;
      else {
        current_sample = audio_buf[cur_audio_buf];
        end_sample = current_sample + data_size[cur_audio_buf];
        // if(current_sample == end_sample) underrun
      }
    }

    buffered_samples += 64;
  }

  int MP3Stream::calc_duration() {
    // decode entire file to get length
    unsigned int samples = 0;

    mp3dec_frame_info_t info = {};

    //while(true)
    while(file_buffer_filled) {
      samples += mp3dec_decode_frame(static_cast<mp3dec_t *>(mp3dec), file_buffer, file_buffer_filled, nullptr, &info);
      read(info.frame_bytes);
    }

    // reset;
    file_offset = 0;
    file_buffer_filled = 0;
    read(0);

    int len_ms = (static_cast<uint64_t>(samples) * 1000) / info.hz;
    return len_ms;
  }

  void MP3Stream::read(int32_t len) {
    // init buffer
    if(!file_buffer_filled) {
      if(file.get_ptr()) {
        // we have the whole thing
        file_buffer = const_cast<uint8_t *>(file.get_ptr());
        file_buffer_filled = file.get_length();
      } else if(!file_buffer) {
        file_buffer = new uint8_t[file_buffer_size];
      }
    }

    // it's a buffer file, nothing to read
    if(file.get_ptr()) {
      file_buffer += len;
      file_buffer_filled -= len;
      return;
    }

    if(len < file_buffer_size)
      memmove(file_buffer, file_buffer + len, file_buffer_filled - len);

    file_buffer_filled -= len;

    auto read = file.read(file_offset, file_buffer_size - file_buffer_filled, reinterpret_cast<char *>(file_buffer) + file_buffer_filled);

    if(read <= 0)
      return;

    file_buffer_filled += read;
    file_offset += read;
  }
}

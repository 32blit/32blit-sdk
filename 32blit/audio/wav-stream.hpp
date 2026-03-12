#pragma once

#include <string>

#include "audio/audio.hpp"
#include "engine/file.hpp"

namespace blit {

  class WavStream final {
  public:

    enum PlayFlags {
      from_start = (1 << 0),
      loop       = (1 << 1)
    };

    WavStream();
    ~WavStream();

    bool load(std::string filename);

    void play(int channel, int flags = 0);
    void pause();
    void restart();

    bool get_playing() const;
    int get_play_flags() const;

    void update();

    unsigned int get_current_sample() const;
    int get_duration_ms() const;

  private:
    void read(int buffer);

    static void static_callback(AudioChannel &channel);
    void callback(AudioChannel &channel);

    // file io
    blit::File file;
    uint32_t file_offset = 0;
    uint32_t file_data_len = 0; // length of data from the header

    static const int file_buffer_size = 1024;
    uint8_t *file_buffers[2] = {nullptr, nullptr};
    int32_t file_buffer_filled[2] = {0, 0};

    int channel = -1;
    int play_flags = 0;

    // decoding
    const uint8_t *data_end, *current_sample;
    uint8_t bits_per_sample;
    uint16_t sample_rate;
    int cur_play_buf = 0;

    unsigned int buffered_samples = 0;
    int duration_ms = 0;
  };
}

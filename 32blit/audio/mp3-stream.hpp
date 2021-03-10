#pragma once

#include <string>

#include "audio/audio.hpp"
#include "engine/file.hpp"

namespace blit {

  class MP3Stream final {
  public:

    enum PlayFlags {
      from_start = (1 << 0),
      loop       = (1 << 1)
    };

    MP3Stream();
    ~MP3Stream();

    bool load(std::string filename, bool do_duration_calc = false);

    void play(int channel, int flags = 0);
    void pause();
    void restart();

    bool get_playing() const;
    int get_play_flags() const;

    void update();

    unsigned int get_current_sample() const;
    int get_duration_ms() const;

  private:
    void decode(int buf_index);
    int calc_duration();

    void read(int32_t len);

    static void static_callback(AudioChannel &channel);
    void callback(AudioChannel &channel);

    // file io
    blit::File file;
    uint32_t file_offset = 0;

    static const int file_buffer_size = 1024 * 4;
    uint8_t *file_buffer = nullptr;
    int32_t file_buffer_filled = 0;

    int channel = -1;
    int play_flags = 0;

    // decoding
    void *mp3dec = nullptr;
    bool need_convert = false;

    static const int audio_buf_size = 1152 * 4;
    int16_t audio_buf[2][audio_buf_size];
    int16_t *current_sample = nullptr, *end_sample = nullptr;
    int data_size[2]{};
    int cur_audio_buf = 0;

    unsigned int buffered_samples = 0;
    int duration_ms = 0;
  };
}


#include <cstring>

#include "mp3.hpp"
#include "audio/mp3-stream.hpp"

#include "assets.hpp"

using namespace blit;

MP3Stream stream;

/* setup */
void init() {
  set_screen_mode(ScreenMode::hires);

  // Files should be encoded as 22050Hz mono, but stereo and/or 44100Hz also work.
  // (The output is still 22050Hz/mono)

  // It's also possible to load directly from the SD card.
  File::add_buffer_file("example.mp3", asset_mp3, asset_mp3_length);

  // Pass false for do_duration_calc if you don't need the duration.
  // (it requires decoding the entire file, which takes a while if reading from the SD card)
  stream.load("example.mp3", true);

  // Any channel can be used here, the others are free for other sounds.
  stream.play(0);
}

void render(uint32_t time) {
  int play_time = stream.get_current_sample() / 22050;

  char buf[100];
  snprintf(buf, 100, "%i/%i seconds", play_time, stream.get_duration_ms() / 1000);

  screen.pen = Pen(0, 0, 0);
  screen.clear();

  screen.pen = Pen(0xFF, 0xFF, 0xFF);
  screen.text(buf, blit::minimal_font, blit::Point(0, 0));
}

void update(uint32_t time) {
  stream.update();
}
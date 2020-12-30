
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
  screen.pen = Pen(0, 0, 0);
  screen.clear();

  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));
  screen.pen = Pen(0, 0, 0);
  screen.text("MP3 Playback", minimal_font, Point(5, 4));

  int play_time = (stream.get_current_sample() * 1000) / 22050;

  // current time / duration
  screen.pen = Pen(255, 255, 255);
  Rect text_rect(5, 30, screen.bounds.w - 10, 10);
  screen.text(std::to_string(play_time / 1000), minimal_font, text_rect);
  screen.text(std::to_string(stream.get_duration_ms() / 1000), minimal_font, text_rect, true, TextAlign::top_right);

  // progress
  screen.pen = Pen(40, 40, 40);
  screen.rectangle(Rect(5, 40, screen.bounds.w - 10, 10));

  screen.pen = Pen(255, 255, 255);
  float w = static_cast<float>(screen.bounds.w - 10) / stream.get_duration_ms() * play_time;
  screen.rectangle(Rect(5, 40, w, 10));
}

void update(uint32_t time) {
  stream.update();
}

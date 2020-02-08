#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "sprite-test.hpp"

using namespace blit;

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;

/* setup */
void init() {
}

int tick_count = 0;
void render(uint32_t time_ms) {
  screen.pen(RGBA(20, 30, 40));
  screen.clear();

  screen.alpha = 255;
  screen.mask = nullptr;

  uint32_t ms_start = now();


  uint32_t ms_end = now();

  // draw FPS meter
  screen.alpha = 255;
  screen.pen(RGBA(255, 255, 255, 100));
  screen.rectangle(Rect(1, 120 - 10, 12, 9));
  screen.pen(RGBA(255, 255, 255, 200));
  std::string fms = std::to_string(ms_end - ms_start);
  screen.text(fms, &minimal_font[0][0], Rect(3, 120 - 9, 10, 16));

  int block_size = 4;
  for (uint32_t i = 0; i < (ms_end - ms_start); i++) {
    screen.pen(RGBA(i * 5, 255 - (i * 5), 0));
    screen.rectangle(Rect(i * (block_size + 1) + 1 + 13, screen.bounds.h - block_size - 1, block_size, block_size));
  }
}

void update(uint32_t time) {

}
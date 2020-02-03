
#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>


#include "rotozoom.hpp"

using namespace blit;

uint8_t logo[16][16] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 0, 0, 0, 0, 1, 0, 2, 0, 1, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 2, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0},
  {0, 0, 2, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0},
  {0, 0, 2, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;

/* setup */
void init() {
  blit::set_screen_mode(blit::hires);
}

void rotozoom(uint32_t time_ms) {
  static float angle = 0.0f;

  static RGBA palette[] = { RGBA(0, 0, 0), RGBA(255, 255, 255), RGBA(0, 255, 0) };

  float c = cos(angle * M_PI / 180.0f);
  float s = sin(angle * M_PI / 180.0f);

  angle += 0.25f;
  angle = angle >= 360.0f ? 0.0f : angle;

  Point p;

  int16_t w = screen.bounds.w;
  int16_t h = screen.bounds.h;

  int16_t hw = w / 2;
  int16_t hh = h / 2;

  uint32_t o = 0;
  for (p.y = 0; p.y < h; p.y++) {
    for (p.x = 0; p.x < w; p.x++) {
      uint8_t u = int16_t(float((p.x - hw) * c - (p.y - hh) * s) * s) & 0b1111;
      uint8_t v = int16_t(float((p.x - hw) * s + (p.y - hh) * c) * s) & 0b1111;

      uint8_t pi = logo[15 - u][v];

      // ~ 19ms/frame
      // screen.pen(palette[pi]);
      // screen.pixel(p);

      // ~ 13ms/frame
      screen.bf((uint8_t *)&palette[pi], &screen, o, 1);
      o++;
    }
  }
}

int tick_count = 0;
void render(uint32_t time_ms) {
  screen.pen(RGBA(0, 0, 0, 255));
  screen.clear();

  tick_count++;

  screen.alpha = 255;
  screen.mask = nullptr;

  uint32_t ms_start = now();

  rotozoom(time_ms);

  uint32_t ms_end = now();

  // draw FPS meter
  screen.pen(RGBA(0, 0, 0, 200));
  screen.rectangle(Rect(5, 5, 20, 16));
  screen.pen(RGBA(255, 0, 0));
  std::string fms = std::to_string(ms_end - ms_start);
  screen.text(fms, &minimal_font[0][0], Rect(10, 10, 10, 16));

  int block_size = 4;
  for (uint32_t i = 0; i < (ms_end - ms_start); i++) {
    screen.pen(RGBA(i * 5, 255 - (i * 5), 0));
    screen.rectangle(Rect(i * (block_size + 1) + 1, screen.bounds.h - block_size - 1, block_size, block_size));
  }

}

void update(uint32_t time) {
}

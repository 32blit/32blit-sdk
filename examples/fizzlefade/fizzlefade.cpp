
#include <string>
#include <memory>
#include <cstdlib>
#include <32blit.hpp>

#include "fizzlefade.hpp"

using namespace blit;

const uint16_t screen_width = 160;
const uint16_t screen_height = 120;

/* define storage for the framebuffer, spritesheet, and mask */
//rgb     __fb[screen_width * screen_height] __attribute__((section(".fb")));
// uint8_t __m[screen_width * screen_height] __attribute__((section(".m")));
// uint8_t __pb[screen_width * screen_height] __attribute__((section(".m")));

/* create surfaces */
//surface fb((uint8_t *)__fb, size(screen_width, screen_height), pixel_format::RGB);
// surface m((uint8_t *)__m, size(screen_width, screen_height), pixel_format::M);
// surface fbb((uint8_t *)__pb, size(screen_width, screen_height), pixel_format::P);

#define FADE_STEPS 3

Pen fade_to[FADE_STEPS]{
  Pen(128, 64, 15, 64),
  Pen(15, 128, 15, 64),
  Pen(15, 64, 128, 64)
};

uint16_t taps[FADE_STEPS]{
  0x74b8,
  0x50e7,
  0x6000
};

int8_t fade_current = 0;

/* setup */
void init() {
  set_screen_mode(ScreenMode::lores);

  screen.alpha = 255;
  screen.mask = nullptr;
  Pen p = fade_to[FADE_STEPS - 1];
  p.a = 255;
  screen.pen = p;
  screen.clear();

  screen.pen = fade_to[0];
}

uint32_t lfsr = 1;
uint16_t tap = 0x74b8;
Point fizzlefade() {
  uint16_t x = lfsr & 0x00ff;
  uint16_t y = (lfsr & 0x7f00) >> 8;

  uint8_t lsb = lfsr & 1;
  lfsr >>= 1;

  if (lsb) {
    lfsr ^= tap;
  }

  if (x - 1 < 160 && y < 120) {
    return Point(x - 1, y);
  }

  return Point(-1, -1);
}

void render(uint32_t time_ms) {
  uint32_t ms_start = now();

  screen.pen = fade_to[fade_current];

  for (int c = 0; c < 500; c++) {
    Point ff = fizzlefade();
    if (ff.x > -1) {
      screen.pixel(ff);
    }
    if (lfsr == 1) {
      fade_current += 1;
      fade_current %= FADE_STEPS;
      tap = taps[fade_current];
      break;
    }
  }

  uint32_t ms_end = now();
  screen.mask = nullptr;
  screen.pen = Pen(255, 0, 0);
  for (uint32_t i = 0; i < (ms_end - ms_start); i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(Rect(i * 3 + 1, 117, 2, 2));
  }
}

void update(uint32_t time) {

}

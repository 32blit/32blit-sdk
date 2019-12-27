#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "sprite-test.hpp"

using namespace blit;

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;

/* define storage for the framebuffer, spritesheet, and mask */
//rgb     __fb[screen_width * screen_height] __attribute__((section(".fb")));
#ifndef __APPLE__
uint8_t __m[screen_width * screen_height] __attribute__((section(".m")));
uint8_t __pb[screen_width * screen_height] __attribute__((section(".fb")));
#else 
uint8_t __m[screen_width * screen_height] __attribute__((section("__DATA,.m")));
uint8_t __pb[screen_width * screen_height] __attribute__((section("__DATA,.fb")));
#endif

/* create surfaces */
//surface fb((uint8_t *)__fb, size(screen_width, screen_height), pixel_format::RGB);
surface m((uint8_t *)__m, pixel_format::M, size(screen_width, screen_height));
surface fbb((uint8_t *)__pb, pixel_format::P, size(screen_width, screen_height));

/* setup */
void init() {
}

int tick_count = 0;
void render(uint32_t time_ms) {
  fb.pen(rgba(20, 30, 40));
  fb.clear();

  fb.alpha = 255;
  fb.mask = nullptr;

  uint32_t ms_start = now();


  uint32_t ms_end = now();

  // draw FPS meter
  fb.alpha = 255;
  fb.pen(rgba(255, 255, 255, 100));
  fb.rectangle(rect(1, 120 - 10, 12, 9));
  fb.pen(rgba(255, 255, 255, 200));
  std::string fms = std::to_string(ms_end - ms_start);
  fb.text(fms, &minimal_font[0][0], rect(3, 120 - 9, 10, 16));

  int block_size = 4;
  for (int i = 0; i < (ms_end - ms_start); i++) {
    fb.pen(rgba(i * 5, 255 - (i * 5), 0));
    fb.rectangle(rect(i * (block_size + 1) + 1 + 13, fb.bounds.h - block_size - 1, block_size, block_size));
  }
}

void update(uint32_t time) {

}
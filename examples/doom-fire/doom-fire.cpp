/*
 * An example that implements the "Doom Fire", as documented by Fabien
 * Sanglard: http://fabiensanglard.net/doom_fire_psx/
 * 
 */
#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "doom-fire.hpp"

using namespace blit;

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;
const uint32_t FPS = 1000 / 27; // DoomFire runs at 27FPS.

rgba pallete[] = { // 36 colours
  rgba(0x07, 0x07, 0x07), rgba(0x1f, 0x07, 0x07), rgba(0x2f, 0x0f, 0x07),
  rgba(0x47, 0x0f, 0x07), rgba(0x57, 0x17, 0x07), rgba(0x67, 0x1f, 0x07),
  rgba(0x77, 0x1f, 0x07), rgba(0x8f, 0x27, 0x07), rgba(0x9f, 0x2f, 0x07),
  rgba(0xaf, 0x3f, 0x07), rgba(0xbf, 0x47, 0x07), rgba(0xc7, 0x47, 0x07),
  rgba(0xDF, 0x4F, 0x07), rgba(0xDF, 0x57, 0x07), rgba(0xDF, 0x57, 0x07),
  rgba(0xD7, 0x5F, 0x07), rgba(0xD7, 0x67, 0x0F), rgba(0xcf, 0x6f, 0x0f),
  rgba(0xcf, 0x77, 0x0f), rgba(0xcf, 0x7f, 0x0f), rgba(0xCF, 0x87, 0x17),
  rgba(0xC7, 0x87, 0x17), rgba(0xC7, 0x8F, 0x17), rgba(0xC7, 0x97, 0x1F),
  rgba(0xBF, 0x9F, 0x1F), rgba(0xBF, 0x9F, 0x1F), rgba(0xBF, 0xA7, 0x27),
  rgba(0xBF, 0xA7, 0x27), rgba(0xBF, 0xAF, 0x2F), rgba(0xB7, 0xAF, 0x2F),
  rgba(0xB7, 0xB7, 0x2F), rgba(0xB7, 0xB7, 0x37), rgba(0xCF, 0xCF, 0x6F),
  rgba(0xDF, 0xDF, 0x9F), rgba(0xEF, 0xEF, 0xC7), rgba(0xFF, 0xFF, 0xFF)   
};

uint8_t screen[screen_width * screen_height];
uint32_t last_time = 0;

uint32_t posAt(uint32_t x, uint32_t y) {
  return y * screen_width + x;
}

void init() {
  blit::set_screen_mode(screen_mode::hires);  

  fb.pen(rgba(0, 0, 0, 255));
  fb.clear();
  
  // Initialises the screen
  for (int x = 0; x < screen_width; x++) {
    for (int y = 0; y < screen_height; y++) {
      uint32_t pos = posAt(x, y);
      screen[pos] = 0;
    }
  }  

  // Initialises the bottom line
  for (int i = 0; i < screen_width; i++) {
    uint32_t pos = posAt(i, screen_height - 1);
    screen[pos] = 35;
  }
}

void update(uint32_t time) {
  if (time - last_time < FPS) {
    return;
  }
  last_time = time;

  for (uint32_t src_y = screen_height; src_y > 1; src_y--) {    
    uint32_t src_row = src_y * screen_width;

    for (uint32_t src_x = 0; src_x < screen_width; src_x++) {
      int rand = std::rand() % 3;    
      uint32_t src_index = src_row - src_x;
      uint8_t color = screen[src_index];
      if (color > 0) {
        color = color - (rand & 1);
      }

      uint32_t dst_x = src_x;

      // Avoid propagating fire to the wrong place, when on the edges
      if (dst_x == 0) {
        dst_x = dst_x + std::max(rand - 1, 0);
      } else if (dst_x == screen_width - 1) {
        dst_x = dst_x + std::min(rand - 1, 0);
      } else {
        dst_x = dst_x + rand - 1;
      }
    
      uint32_t dst_index = src_row - screen_width - dst_x;
      screen[dst_index] = color; 
    }
  }
}

void render(uint32_t time) {
  for (uint32_t x = 0; x < screen_width; x++) {
    for (uint32_t y = 0; y < screen_height; y++) {
      uint32_t pos = posAt(x, y);
      fb.pen(pallete[screen[pos]]);
      fb.pixel(point(x, y));
    }
  }
}

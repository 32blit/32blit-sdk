/*
 * An example that implements the "Doom Fire", as documented by Fabien
 * Sanglard: http://fabiensanglard.net/doom_fire_psx/
 * 
 */
#include <string>
#include <cstring>
#include <memory>
#include <cstdlib>

#include "doom-fire.hpp"

using namespace blit;

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;
const uint32_t FPS = 1000 / 27; // DoomFire runs at 27FPS.

Pen pallete[] = { // 36 colours
  Pen(0x07, 0x07, 0x07), Pen(0x1f, 0x07, 0x07), Pen(0x2f, 0x0f, 0x07),
  Pen(0x47, 0x0f, 0x07), Pen(0x57, 0x17, 0x07), Pen(0x67, 0x1f, 0x07),
  Pen(0x77, 0x1f, 0x07), Pen(0x8f, 0x27, 0x07), Pen(0x9f, 0x2f, 0x07),
  Pen(0xaf, 0x3f, 0x07), Pen(0xbf, 0x47, 0x07), Pen(0xc7, 0x47, 0x07),
  Pen(0xDF, 0x4F, 0x07), Pen(0xDF, 0x57, 0x07), Pen(0xDF, 0x57, 0x07),
  Pen(0xD7, 0x5F, 0x07), Pen(0xD7, 0x67, 0x0F), Pen(0xcf, 0x6f, 0x0f),
  Pen(0xcf, 0x77, 0x0f), Pen(0xcf, 0x7f, 0x0f), Pen(0xCF, 0x87, 0x17),
  Pen(0xC7, 0x87, 0x17), Pen(0xC7, 0x8F, 0x17), Pen(0xC7, 0x97, 0x1F),
  Pen(0xBF, 0x9F, 0x1F), Pen(0xBF, 0x9F, 0x1F), Pen(0xBF, 0xA7, 0x27),
  Pen(0xBF, 0xA7, 0x27), Pen(0xBF, 0xAF, 0x2F), Pen(0xB7, 0xAF, 0x2F),
  Pen(0xB7, 0xB7, 0x2F), Pen(0xB7, 0xB7, 0x37), Pen(0xCF, 0xCF, 0x6F),
  Pen(0xDF, 0xDF, 0x9F), Pen(0xEF, 0xEF, 0xC7), Pen(0xFF, 0xFF, 0xFF)   
};

uint8_t fire[screen_width * screen_height];
uint32_t last_time = 0;
int8_t wind = 0;
bool enabled = true;

uint32_t posAt(uint32_t x, uint32_t y) {
  return y * screen_width + x;
}

void init() {
  blit::set_screen_mode(ScreenMode::hires);  

  screen.pen = Pen(0, 0, 0, 255);
  screen.clear();
  
  // Initialises the screen
  for (int x = 0; x < screen_width; x++) {
    for (int y = 0; y < screen_height; y++) {
      uint32_t pos = posAt(x, y);
      fire[pos] = 0;
    }
  }  

  // Initialises the bottom line
  for (int i = 0; i < screen_width; i++) {
    uint32_t pos = posAt(i, screen_height - 1);
    fire[pos] = 35;
  }
}

void update(uint32_t time) {
  // Adds wind to east.
  if (buttons.pressed & Button::DPAD_LEFT) {
    wind--;
  }

  // Adds wind to east.
  if (buttons.pressed & Button::DPAD_RIGHT) {
    wind++;
  }

  // Toggles fire on and off.
  if (buttons.pressed & Button::X) {
    uint8_t fire_index = enabled ? 0 : 35;
    for (int i = 0; i < screen_width; i++) {
      uint32_t pos = posAt(i, screen_height - 1);
      fire[pos] = fire_index;
    }
    enabled = !enabled;
  }
}

void render(uint32_t time) {
  if (time - last_time < FPS) {
    return;
  }
  last_time = time;
  for (int y = 0; y < screen_height; y++) {
    // Precompute the rows for a small performance gain.
    int row = y * screen_width;

    // For each pixel in each row, calculate the colours that will be
    // rendered on the previous row, on the next call to update.
    int next_row = y == 0 ? 0 : (y - 1) * screen_width;

    for (int x = 0; x < screen_width; x++) {
      // Draw the current pixel.
      uint8_t color = fire[row + x];
      screen.pen = pallete[color];
      screen.pixel(Point(x, y));

      // Update the pixels on the previous row.
      if (y > 0) {
        int new_x = x;
        int rand = std::rand() % 3;
        new_x = (new_x + rand - 1 + wind);
        if (new_x >= screen_width) {
          new_x = new_x - screen_width;
        }
        else if (new_x < 0) {
          new_x = new_x + screen_width;
        }
        color = color > 0 ? color - (rand & 1) : 0;
        fire[next_row + new_x] = color;
      }
    }
  }
}

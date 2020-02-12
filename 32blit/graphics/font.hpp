#pragma once

#include <stdint.h>

namespace blit {
  struct Font {
    Font(const uint8_t *data, uint8_t char_w, uint8_t char_h, uint8_t spacing_x = 2, uint8_t spacing_y = 1) : data(data), char_w(char_w), char_h(char_h), spacing_x(spacing_x), spacing_y(spacing_y) {}

    const uint8_t *data;
    uint8_t char_w, char_h;
    uint8_t spacing_x, spacing_y;
  };

  extern const Font outline_font;
  extern const Font fat_font;
  extern const Font minimal_font;
}
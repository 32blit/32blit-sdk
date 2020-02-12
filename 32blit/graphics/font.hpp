#pragma once

#include <stdint.h>

namespace blit {
  struct Font {
    Font(const uint8_t *data, uint8_t char_w, uint8_t char_h) : data(data), char_w(char_w), char_h(char_h) {}

    const uint8_t *data;
    uint8_t char_w, char_h;
  };

  extern const Font outline_font;
  extern const Font fat_font;
  extern const Font minimal_font;
}
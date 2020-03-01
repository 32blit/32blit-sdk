#pragma once

#include <cstdint>

namespace blit {
  struct Font {
    constexpr Font(const uint8_t *data, const uint8_t *char_w_variable, uint8_t char_w, uint8_t char_h, uint8_t spacing_y = 1)
      : data(data), char_w(char_w), char_h(char_h), spacing_y(spacing_y), char_w_variable(char_w_variable) {}
    
    constexpr Font(const uint8_t *data) : data(data + 8 + data[4]), char_w(data[5]), char_h(data[6]), spacing_y(data[7]), char_w_variable(data + 8) {}

    const uint8_t *data;

    // fixed size
    uint8_t char_w, char_h;
    uint8_t spacing_y;

    // variable width
    const uint8_t *char_w_variable;
  };

  extern const Font outline_font;
  extern const Font fat_font;
  extern const Font minimal_font;
}

#pragma once

#include <stdint.h>

namespace blit {
  struct Font {
    Font(const uint8_t *data) : data(data) {}

    const uint8_t *data;
  };

  extern const Font outline_font;
  extern const Font fat_font;
  extern const Font minimal_font;
}
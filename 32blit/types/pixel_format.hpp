#pragma once

#include <stdint.h>

namespace blit {

  enum class PixelFormat {
    RGB     = 0,   // red, green, blue (8-bits per channel)
    RGBA    = 1,   // red, green, blue, alpha (8-bits per channel)
    P       = 2,   // palette entry (8-bits) into attached palette
    M       = 3    // mask (8-bits, single channel)
  };

  static const uint8_t pixel_format_stride[] = {
    3,             // RGB
    4,             // RGBA
    1,             // P
    1,             // M
  };

  #pragma pack(push, 1)
  struct Pen {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Pen() : a(0), r(0), g(0), b(0) {}
    Pen(int a) : a(a), r(0), g(0), b(0) {}
    Pen(float a) : a((uint8_t)(a * 255.0f)), r(0), g(0), b(0) {}
    Pen(int r, int g, int b, int a = 255) : a(a), r(r), g(g), b(b) {}
    Pen(float r, float g, float b, float a = 1.0f) : a((uint8_t)(a * 255.0f)), r((uint8_t)(r * 255.0f)), g((uint8_t)(g * 255.0f)), b((uint8_t)(b * 255.0f)) {}
  };
  #pragma pack(pop)

};
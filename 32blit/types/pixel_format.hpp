#pragma once

#include <stdint.h>

namespace blit {

  enum pixel_format {
    RGBA   = 0,    // red, green, blue, alpha (8-bits per channel)
    RGB    = 1,    // red, green, blue (8-bits per channel)
    RGB565 = 2,    // red, green, blue (5/6/5-bits per channel)
    P      = 3,    // palette entry (8-bits) into attached palette
    M      = 4     // mask value (8-bits)
  };

  static const uint8_t pixel_format_stride[] = {
    4,             // pixel_format::RGBA
    3,             // pixel_format::RGB
    2,             // pixel_format::RGB565
    1,             // pixel_format::P
    1              // pixel_format::M
  };

  #pragma pack(push, 1)
  struct rgba {
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;

    rgba() : a(0), r(0), g(0), b(0) {}
    rgba(int a) : a(a), r(0), g(0), b(0) {}
    rgba(float a) : a((uint8_t)(a * 255.0f)), r(0), g(0), b(0) {}
    rgba(int r, int g, int b, int a = 255) : a(a), r(r), g(g), b(b) {}
    rgba(float r, float g, float b, float a = 1.0f) : a((uint8_t)(a * 255.0f)), r((uint8_t)(r * 255.0f)), g((uint8_t)(g * 255.0f)), b((uint8_t)(b * 255.0f)) {}
  };
  #pragma pack(pop)

  #pragma pack(push, 1)
  struct rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    rgb() : r(0), g(0), b(0) {}
    rgb(int r, int g, int b) : r(r), g(g), b(b) {}
    rgb(float r, float g, float b) : r((uint8_t)(r * 255.0f)), g((uint8_t)(g * 255.0f)), b((uint8_t)(b * 255.0f)) {}
  };
  #pragma pack(pop)

  #pragma pack(push, 1)
  struct rgb565 {
    uint16_t v;

    rgb565() { r(0); g(0); b(0); }
    rgb565(int nr, int ng, int nb) { r(nr); g(ng); b(nb); }
    rgb565(float nr, float ng, float nb) { r((uint8_t)(nr * 255.0f)); g((uint8_t)(ng * 255.0f)); b((uint8_t)(nb * 255.0f)); }

    uint8_t r() { return (v >> 8) & 0b11111000; }
    uint8_t g() { return (v >> 3) & 0b11111100; }
    uint8_t b() { return (v << 3) & 0b11111000; }

    void r(uint8_t r) { v &= 0b0000011111111111; v |= (r >> 3) << 11; }
    void g(uint8_t g) { v &= 0b1111100000011111; v |= (g >> 2) <<  5; }
    void b(uint8_t b) { v &= 0b1111111111100000; v |= (b >> 3) <<  0; }
  };
  #pragma pack(pop)

};
#pragma once

#include "../types/pixel_format.hpp"
#include "surface.hpp"

namespace blit {

  // All sprite mirroring and rotations (90/180/270) can be composed
  // of simple horizontal/vertical flips and x/y coordinate swaps.
  //
  // The bits set represent the transforms required to achieve the
  // end result. Operations are performed (if needed) in the following 
  // order: horizontal flip -> vertical flip -> x/y swap
  // 
  // For example a 90 degree rotation needs a vertical flip
  // followed by an x/y coordinate swap.
  enum sprite_transform {
    NONE = 0b000,
    HORIZONTAL = 0b001,
    VERTICAL = 0b010,
    XYSWAP = 0b100,
    R90 = 0b101,
    R180 = 0b011,
    R270 = 0b110
  };

  struct spritesheet : surface {
    uint16_t  rows, cols;

    spritesheet(uint8_t *data, pixel_format format, const packed_image *image);

    static spritesheet *load(const uint8_t *data, uint8_t *buffer = nullptr);
    static spritesheet *load(const packed_image *image, uint8_t *buffer = nullptr);

    rect sprite_bounds(const uint16_t &index);          
    rect sprite_bounds(const point &p);
    rect sprite_bounds(const rect &r);
  };

} 
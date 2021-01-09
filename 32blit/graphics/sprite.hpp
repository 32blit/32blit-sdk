#pragma once

#include "surface.hpp"

namespace blit {
  using SpriteSheet [[deprecated]] = Surface;

  /** 
   * All sprite mirroring and rotations (90/180/270) can be composed
   * of simple horizontal/vertical flips and x/y coordinate swaps.
   *
   * The bits set represent the transforms required to achieve the
   * end result. Operations are performed (if needed) in the following 
   * order: horizontal flip -> vertical flip -> x/y swap
   * 
   * For example a 90 degree rotation needs a vertical flip
   * followed by an x/y coordinate swap.
  */
  enum SpriteTransform {
    NONE = 0b000,
    HORIZONTAL = 0b001,
    VERTICAL = 0b010,
    XYSWAP = 0b100,
    R90 = 0b101,
    R180 = 0b011,
    R270 = 0b110
  };

} 
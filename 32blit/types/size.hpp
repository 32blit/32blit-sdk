#pragma once

#include <stdint.h>

#include "point.hpp"

namespace blit {

  struct size {

    int32_t w, h;

    size() : w(0), h(0) {}
    size(int32_t w, int32_t h) : w(w), h(h) {}

    inline size& operator*= (const float a) { w *= a;   h *= a;   return *this; }

    bool empty() { return w <= 0 || h <= 0; }
    
    int32_t area() { return w * h; }

    bool contains(const point &p) {
      return p.x >= 0 && p.y >= 0 && p.x < w && p.y < h;
    }

  };

  inline size operator*  (size lhs, const float a) { lhs *= a; return lhs; }

}
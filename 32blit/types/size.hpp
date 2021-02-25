#pragma once

#include <cstdint>

#include "point.hpp"

namespace blit {

  struct Size {

    int32_t w = 0, h = 0;

    Size() = default;
    constexpr Size(int32_t w, int32_t h) : w(w), h(h) {}

    inline Size& operator*= (const float a) { w = static_cast<int32_t>(w * a); h = static_cast<int32_t>(h * a); return *this; }
    inline Size& operator/= (const float a) { w = static_cast<int32_t>(w / a); h = static_cast<int32_t>(h / a); return *this; }
    inline Size& operator*= (const int a) { w = static_cast<int32_t>(w * a); h = static_cast<int32_t>(h * a); return *this; }
    inline Size& operator/= (const int a) { w = static_cast<int32_t>(w / a); h = static_cast<int32_t>(h / a); return *this; }

    constexpr bool empty() const { return w <= 0 || h <= 0; }

    constexpr int32_t area() const { return w * h; }

    constexpr bool contains(const Point &p) const {
      return p.x >= 0 && p.y >= 0 && p.x < w && p.y < h;
    }

  };

  inline Size operator/ (Size lhs, const float a) { lhs /= a; return lhs;}
  inline Size operator/ (Size lhs, const int a) { lhs /= a; return lhs; }
  inline Size operator* (Size lhs, const float a) { lhs *= a; return lhs; }
  inline Size operator* (Size lhs, const int a) {  lhs *= a; return lhs; }
  inline bool operator== (Size lhs, Size rhs) { return lhs.w == rhs.w && lhs.h == rhs.h; }
  inline bool operator!= (Size lhs, Size rhs) { return lhs.w != rhs.w || lhs.h != rhs.h; }

}

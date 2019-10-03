#pragma once

#include "mat3.hpp"
#include "vec2.hpp"

namespace blit {

  struct point {
    int32_t x, y;

    point() : x(0), y(0) {}
    point(const point &p) : x(p.x), y(p.y) {}
    point(int32_t x, int32_t y) : x(x), y(y) {}
    point(vec2 v) : x(int32_t(v.x)), y(int32_t(v.y)) {}

    inline point& operator-= (const point &a) { x -= a.x; y -= a.y; return *this; }
    inline point& operator+= (const point &a) { x += a.x; y += a.y; return *this; }
    inline point& operator*= (const float a) { x *= a;   y *= a;   return *this; }
    inline point& operator*= (const mat3 &a) { this->transform(a); return *this; }
    inline point& operator/= (const int32_t a) { x /= a;   y /= a;   return *this; }

    void   transform(const mat3 &m) {     
      float tx = x; float ty = y;
      this->x = (m.v00 * tx + m.v01 * ty + m.v02);
      this->y = (m.v10 * tx + m.v11 * ty + m.v12);
    }
  };

  inline point operator-  (point lhs, const point &rhs) { lhs -= rhs; return lhs; }
  inline point operator-  (const point &rhs) { return point(-rhs.x, -rhs.y); }
  inline point operator+  (point lhs, const point &rhs) { lhs += rhs; return lhs; }
  inline point operator*  (point lhs, const float a) { lhs *= a; return lhs; }
  inline point operator*  (point lhs, const mat3 &a) { lhs *= a; return lhs; }
  inline point operator/  (point lhs, const int32_t a) { lhs /= a; return lhs; }

}
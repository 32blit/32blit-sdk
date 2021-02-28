#pragma once

#include "mat3.hpp"
#include "vec2.hpp"

namespace blit {

  struct Point {
    int32_t x = 0, y = 0;

    Point() = default;
    Point(const Point &p) = default;
    constexpr Point(int32_t x, int32_t y) : x(x), y(y) {}
    constexpr Point(Vec2 v) : x(int32_t(v.x)), y(int32_t(v.y)) {}

    explicit constexpr operator Vec2() const {return Vec2(x, y);}

    inline Point& operator-= (const Point &a) { x -= a.x; y -= a.y; return *this; }
    inline Point& operator+= (const Point &a) { x += a.x; y += a.y; return *this; }
    inline Point& operator*= (const float a) { x = static_cast<int32_t>(x * a); y = static_cast<int32_t>(y * a); return *this; }
    inline Point& operator*= (const Mat3 &a) { this->transform(a); return *this; }
    inline Point& operator/= (const int32_t a) { x /= a;   y /= a;   return *this; }

    void transform(const Mat3 &m) {
      auto tx = static_cast<float>(x); auto ty = static_cast<float>(y);
      this->x = static_cast<int32_t>(m.v00 * tx + m.v01 * ty + m.v02);
      this->y = static_cast<int32_t>(m.v10 * tx + m.v11 * ty + m.v12);
    }
  };

  inline bool operator== (const Point &lhs, const Point &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
  inline bool operator!= (const Point &lhs, const Point &rhs) { return !(lhs == rhs); }
  inline Point operator-  (Point lhs, const Point &rhs) { lhs -= rhs; return lhs; }
  inline Point operator-  (const Point &rhs) { return Point(-rhs.x, -rhs.y); }
  inline Point operator+  (Point lhs, const Point &rhs) { lhs += rhs; return lhs; }
  inline Point operator*  (Point lhs, const float a) { lhs *= a; return lhs; }
  inline Point operator*  (Point lhs, const Mat3 &a) { lhs *= a; return lhs; }
  inline Point operator/  (Point lhs, const int32_t a) { lhs /= a; return lhs; }

}

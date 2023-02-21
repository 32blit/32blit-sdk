/*! \file vec2.cpp
*/
#include <cstring>

#include "vec2.hpp"
#include "mat3.hpp"

namespace blit {

  void Vec2::rotate(float a) {
    float c = cosf(a);
    float s = sinf(a);
    float rx = this->x * c - this->y * s;
    float ry = this->x * s + this->y * c;
    this->x = rx;
    this->y = ry;
  }


  void Vec2::transform(const Mat3 &m) {
    float tx = x; float ty = y;
    this->x = (m.v00 * tx + m.v01 * ty + m.v02);
    this->y = (m.v10 * tx + m.v11 * ty + m.v12);
  }

  float Vec2::angle(Vec2 o) {
    return std::atan2(this->x * o.y - this->y * o.x, this->x * o.x + this->y * o.y);
  }

  float Vec2::angle_to(Vec2 o) {
    return std::atan2(this->y - o.y, this->x - o.x);
  }

}
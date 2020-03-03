/*! \file mat3.cpp
*/
#include <cmath>

#undef M_PI
#define M_PI           3.14159265358979323846f  /* pi */

#include "mat3.hpp"
#include "vec2.hpp"

Mat3::Mat3() {
  v00 = 0.0f; v10 = 0.0f; v20 = 0.0f;
  v01 = 0.0f; v11 = 0.0f; v21 = 0.0f;
  v02 = 0.0f; v12 = 0.0f; v22 = 0.0f;
}

Mat3::Mat3(const Mat3 &m) = default;

Mat3 Mat3::identity() {
  Mat3 m;
  m.v00 = 1.0f; m.v11 = 1.0f; m.v22 = 1.0f;
  return m;
}

Mat3 Mat3::rotation(float a) {
  float c = cosf(a);
  float s = sinf(a);

  Mat3 r = Mat3::identity();

  r.v00 = c;
  r.v01 = s;
  r.v10 = -s;
  r.v11 = c;

  return r;
}

Mat3 Mat3::translation(Vec2 v) {
  Mat3 r = Mat3::identity();
  r.v02 = v.x; r.v12 = v.y;
  return r;
}

Mat3 Mat3::scale(Vec2 v) {
  Mat3 r = Mat3::identity();
  r.v00 = v.x; r.v11 = v.y;
  return r;
}

void Mat3::inverse() {
  Mat3 m(*this);

  float invdet = 1.0f / 
    ( m.v00 * (m.v11 * m.v22 - m.v21 * m.v12) -
      m.v01 * (m.v10 * m.v22 - m.v12 * m.v20) +
      m.v02 * (m.v10 * m.v21 - m.v11 * m.v20) );
  
  this->v00 = (m.v11 * m.v22 - m.v21 * m.v12) * invdet;
  this->v01 = (m.v02 * m.v21 - m.v01 * m.v22) * invdet;
  this->v02 = (m.v01 * m.v12 - m.v02 * m.v11) * invdet;
  this->v10 = (m.v12 * m.v20 - m.v10 * m.v22) * invdet;
  this->v11 = (m.v00 * m.v22 - m.v02 * m.v20) * invdet;
  this->v12 = (m.v10 * m.v02 - m.v00 * m.v12) * invdet;
  this->v20 = (m.v10 * m.v21 - m.v20 * m.v11) * invdet;
  this->v21 = (m.v20 * m.v01 - m.v00 * m.v21) * invdet;
  this->v22 = (m.v00 * m.v11 - m.v10 * m.v01) * invdet;
}


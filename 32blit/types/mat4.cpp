/*! \file mat4.cpp
*/
#include <math.h>
#define M_PI           3.14159265358979323846f  /* pi */

#include "mat4.hpp"
#include "vec3.hpp"

mat4::mat4() {
  v00 = 0.0f; v10 = 0.0f; v20 = 0.0f; v30 = 0.0f;
  v01 = 0.0f; v11 = 0.0f; v21 = 0.0f; v31 = 0.0f;
  v02 = 0.0f; v12 = 0.0f; v22 = 0.0f; v32 = 0.0f;
  v03 = 0.0f; v13 = 0.0f; v23 = 0.0f; v33 = 0.0f;
}

mat4::mat4(const mat4 &m) = default;

mat4 mat4::identity() {
  mat4 m;
  m.v00 = 1.0f; m.v11 = 1.0f; m.v22 = 1.0f; m.v33 = 1.0f;
  return m;
}

mat4 mat4::rotation(float a, vec3 v) {
  v.normalize();

  a *= (M_PI / 180.0f);

  float c = cosf(a);
  float s = sinf(a);
  float t = 1.0f - c;

  mat4 r = mat4::identity();

  r.v00 = v.x * v.x * t + c;
  r.v01 = v.x * v.y * t - v.z * s;
  r.v02 = v.x * v.z * t + v.y * s;
  r.v10 = v.y * v.x * t + v.z * s;
  r.v11 = v.y * v.y * t + c;
  r.v12 = v.y * v.z * t - v.x * s;
  r.v20 = v.z * v.x * t - v.y * s;
  r.v21 = v.z * v.y * t + v.x * s;
  r.v22 = v.z * v.z * t + c;

  return r;
}

mat4 mat4::translation(vec3 v) {
  mat4 r = mat4::identity();
  r.v03 = v.x; r.v13 = v.y; r.v23 = v.z;
  return r;
}

mat4 mat4::scale(vec3 v) {
  mat4 r = mat4::identity();
  r.v00 = v.x; r.v11 = v.y; r.v22 = v.z;
  return r;
}

void mat4::inverse() {
  mat4 m(*this);

  float s0 = m.v00 * m.v11 - m.v10 * m.v01;
  float s1 = m.v00 * m.v12 - m.v10 * m.v02;
  float s2 = m.v00 * m.v13 - m.v10 * m.v03;
  float s3 = m.v01 * m.v12 - m.v11 * m.v02;
  float s4 = m.v01 * m.v13 - m.v11 * m.v03;
  float s5 = m.v02 * m.v13 - m.v12 * m.v03;
  float c5 = m.v22 * m.v33 - m.v32 * m.v23;
  float c4 = m.v21 * m.v33 - m.v31 * m.v23;
  float c3 = m.v21 * m.v32 - m.v31 * m.v22;
  float c2 = m.v20 * m.v33 - m.v30 * m.v23;
  float c1 = m.v20 * m.v32 - m.v30 * m.v22;
  float c0 = m.v20 * m.v31 - m.v30 * m.v21;

  float invdet = 1.0f / (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);

  this->v00 = (m.v11 * c5 - m.v12 * c4 + m.v13 * c3) * invdet;
  this->v01 = (-m.v01 * c5 + m.v02 * c4 - m.v03 * c3) * invdet;
  this->v02 = (m.v31 * s5 - m.v32 * s4 + m.v33 * s3) * invdet;
  this->v03 = (-m.v21 * s5 + m.v22 * s4 - m.v23 * s3) * invdet;
  this->v10 = (-m.v10 * c5 + m.v12 * c2 - m.v13 * c1) * invdet;
  this->v11 = (m.v00 * c5 - m.v02 * c2 + m.v03 * c1) * invdet;
  this->v12 = (-m.v30 * s5 + m.v32 * s2 - m.v33 * s1) * invdet;
  this->v13 = (m.v20 * s5 - m.v22 * s2 + m.v23 * s1) * invdet;
  this->v20 = (m.v10 * c4 - m.v11 * c2 + m.v13 * c0) * invdet;
  this->v21 = (-m.v00 * c4 + m.v01 * c2 - m.v03 * c0) * invdet;
  this->v22 = (m.v30 * s4 - m.v31 * s2 + m.v33 * s0) * invdet;
  this->v23 = (-m.v20 * s4 + m.v21 * s2 - m.v23 * s0) * invdet;
  this->v30 = (-m.v10 * c3 + m.v11 * c1 - m.v12 * c0) * invdet;
  this->v31 = (m.v00 * c3 - m.v01 * c1 + m.v02 * c0) * invdet;
  this->v32 = (-m.v30 * s3 + m.v31 * s1 - m.v32 * s0) * invdet;
  this->v33 = (m.v20 * s3 - m.v21 * s1 + m.v22 * s0) * invdet;
}


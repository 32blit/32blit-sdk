#pragma once

struct vec2;

struct mat3 {
  float v00, v10, v20;
  float v01, v11, v21;
  float v02, v12, v22;

  mat3();
  mat3(const mat3 &m);

  inline  mat3& operator*= (const mat3 &m) {        
    float r00 = this->v00 * m.v00 + this->v01 * m.v10 + this->v02 * m.v20;
    float r01 = this->v00 * m.v01 + this->v01 * m.v11 + this->v02 * m.v21;
    float r02 = this->v00 * m.v02 + this->v01 * m.v12 + this->v02 * m.v22;
    float r10 = this->v10 * m.v00 + this->v11 * m.v10 + this->v12 * m.v20;
    float r11 = this->v10 * m.v01 + this->v11 * m.v11 + this->v12 * m.v21;
    float r12 = this->v10 * m.v02 + this->v11 * m.v12 + this->v12 * m.v22;
    float r20 = this->v20 * m.v00 + this->v21 * m.v10 + this->v22 * m.v20;
    float r21 = this->v20 * m.v01 + this->v21 * m.v11 + this->v22 * m.v21;
    float r22 = this->v20 * m.v02 + this->v21 * m.v12 + this->v22 * m.v22;
    
    this->v00 = r00; this->v01 = r01; this->v02 = r02;
    this->v10 = r10; this->v11 = r11; this->v12 = r12;
    this->v20 = r20; this->v21 = r21; this->v22 = r22;

    return *this;
  }

  static mat3 identity();
  static mat3 rotation(float a);
  static mat3 translation(vec2 v);
  static mat3 scale(vec2 v);
  void inverse();
};

inline mat3 operator*  (mat3 lhs, const mat3 &m) { lhs *= m; return lhs; }
#pragma once

namespace blit {

  struct Vec3;

  struct Mat4 {
    float v00, v10, v20, v30;
    float v01, v11, v21, v31;
    float v02, v12, v22, v32;
    float v03, v13, v23, v33;

    Mat4();
    Mat4(const Mat4 &m);


    inline  Mat4& operator*= (const Mat4 &m) {
      float r00 = this->v00 * m.v00 + this->v01 * m.v10 + this->v02 * m.v20 + this->v03 * m.v30;
      float r01 = this->v00 * m.v01 + this->v01 * m.v11 + this->v02 * m.v21 + this->v03 * m.v31;
      float r02 = this->v00 * m.v02 + this->v01 * m.v12 + this->v02 * m.v22 + this->v03 * m.v32;
      float r03 = this->v00 * m.v03 + this->v01 * m.v13 + this->v02 * m.v23 + this->v03 * m.v33;
      float r10 = this->v10 * m.v00 + this->v11 * m.v10 + this->v12 * m.v20 + this->v13 * m.v30;
      float r11 = this->v10 * m.v01 + this->v11 * m.v11 + this->v12 * m.v21 + this->v13 * m.v31;
      float r12 = this->v10 * m.v02 + this->v11 * m.v12 + this->v12 * m.v22 + this->v13 * m.v32;
      float r13 = this->v10 * m.v03 + this->v11 * m.v13 + this->v12 * m.v23 + this->v13 * m.v33;
      float r20 = this->v20 * m.v00 + this->v21 * m.v10 + this->v22 * m.v20 + this->v23 * m.v30;
      float r21 = this->v20 * m.v01 + this->v21 * m.v11 + this->v22 * m.v21 + this->v23 * m.v31;
      float r22 = this->v20 * m.v02 + this->v21 * m.v12 + this->v22 * m.v22 + this->v23 * m.v32;
      float r23 = this->v20 * m.v03 + this->v21 * m.v13 + this->v22 * m.v23 + this->v23 * m.v33;
      float r30 = this->v30 * m.v00 + this->v31 * m.v10 + this->v32 * m.v20 + this->v33 * m.v30;
      float r31 = this->v30 * m.v01 + this->v31 * m.v11 + this->v32 * m.v21 + this->v33 * m.v31;
      float r32 = this->v30 * m.v02 + this->v31 * m.v12 + this->v32 * m.v22 + this->v33 * m.v32;
      float r33 = this->v30 * m.v03 + this->v31 * m.v13 + this->v32 * m.v23 + this->v33 * m.v33;

      this->v00 = r00; this->v01 = r01; this->v02 = r02; this->v03 = r03;
      this->v10 = r10; this->v11 = r11; this->v12 = r12; this->v13 = r13;
      this->v20 = r20; this->v21 = r21; this->v22 = r22; this->v23 = r23;
      this->v30 = r30; this->v31 = r31; this->v32 = r32; this->v33 = r33;

      return *this;
    }

    static Mat4 identity();
    static Mat4 rotation(float a, Vec3 v);
    static Mat4 translation(Vec3 v);
    static Mat4 scale(Vec3 v);
    static Mat4 ortho(float l, float r, float b, float t, float n, float f);
    void inverse();
    Vec3 transform(Vec3 a);
  };

  inline Mat4 operator*  (Mat4 lhs, const Mat4 &m) { lhs *= m; return lhs; }
}

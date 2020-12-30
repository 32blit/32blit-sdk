#pragma once
/*
extern "C" {
  #include <lua\lua.h>
  #include <lua\lualib.h>
  #include <lua\lauxlib.h>
}*/

namespace blit {

  struct Mat4;

  struct Vec3 {
    float x;
    float y;
    float z;

    Vec3(const Vec3 &v) = default;
    constexpr Vec3(const float x = 0, const float y = 0, const float z = 0) : x(x), y(y), z(z) {}

    inline Vec3& operator-= (const Vec3 &a) { x -= a.x; y -= a.y;  z -= a.z; return *this; }
    inline Vec3& operator+= (const Vec3 &a) { x += a.x; y += a.y;  z += a.z; return *this; }
    inline Vec3& operator*= (const float a) { x *= a;   y *= a;    z *= a;   return *this; }
    inline Vec3& operator*= (const Mat4 &a) { this->transform(a); return *this; }
    inline Vec3& operator*= (const Vec3 &a) { x *= a.x;   y *= a.y;    z *= a.z;   return *this; }
    inline Vec3& operator/= (const float a) { x /= a;   y /= a;    z /= a;   return *this; }
    inline Vec3& operator/= (const Vec3 &a) { x /= a.x;   y /= a.y;    z /= a.z;   return *this; }

    void   transform(const Mat4 &m);
    void   normalize();
    float  length();
    Vec3   cross(const Vec3 &a);
    Vec3   cross(const Vec3 *a);
    float  dot(const Vec3 &a);
    float  dot(const Vec3 *a);
  };

  inline Vec3 operator-  (Vec3 lhs, const Vec3 &rhs) { lhs -= rhs; return lhs; }
  inline Vec3 operator-  (const Vec3 &rhs) { return Vec3(-rhs.x, -rhs.y, -rhs.z); }
  inline Vec3 operator+  (Vec3 lhs, const Vec3 &rhs) { lhs += rhs; return lhs; }
  inline Vec3 operator*  (Vec3 lhs, const float a) { lhs *= a; return lhs; }
  inline Vec3 operator*  (Vec3 lhs, const Mat4 &a) { lhs *= a; return lhs; }
  inline Vec3 operator*  (Vec3 lhs, const Vec3 &rhs) { lhs *= rhs; return lhs; }
  inline Vec3 operator/  (Vec3 lhs, const float a) { lhs /= a; return lhs; }
  inline Vec3 operator/  (Vec3 lhs, const Vec3 &rhs) { lhs /= rhs; return lhs; }

  /*
  void register_vec3lib(lua_State *L);
  vec3 *push_vec3(lua_State* L, float x = 0, float y = 0, float z = 0);
  vec3 *push_vec3(lua_State* L, const vec3 &v);*/
}
#pragma once
/*
extern "C" {
  #include <lua\lua.h>
  #include <lua\lualib.h>
  #include <lua\lauxlib.h>
}*/

struct mat4;

struct vec3 {
  float x;
  float y;
  float z;  

  vec3(const vec3 &v);
  vec3(const float x = 0, const float y = 0, const float z = 0);

  inline vec3& operator-= (const vec3 &a) { x -= a.x; y -= a.y;  z -= a.z; return *this; }
  inline vec3& operator+= (const vec3 &a) { x += a.x; y += a.y;  z += a.z; return *this; }
  inline vec3& operator*= (const float a) { x *= a;   y *= a;    z *= a;   return *this; }
  inline vec3& operator*= (const mat4 &a) { this->transform(a); return *this; }
  inline vec3& operator*= (const vec3 &a) { x *= a.x;   y *= a.y;    z *= a.z;   return *this; }
  inline vec3& operator/= (const float a) { x /= a;   y /= a;    z /= a;   return *this; }
  inline vec3& operator/= (const vec3 &a) { x /= a.x;   y /= a.y;    z /= a.z;   return *this; }
  
  void   transform(const mat4 &m);
  void   normalize();
  float  length();
  vec3   cross(const vec3 &a);
  vec3   cross(const vec3 *a);
  float  dot(const vec3 &a);
  float  dot(const vec3 *a);
};

inline vec3 operator-  (vec3 lhs, const vec3 &rhs) { lhs -= rhs; return lhs; }
inline vec3 operator-  (const vec3 &rhs) { return vec3(-rhs.x, -rhs.y, -rhs.z); }
inline vec3 operator+  (vec3 lhs, const vec3 &rhs) { lhs += rhs; return lhs; }
inline vec3 operator*  (vec3 lhs, const float a) { lhs *= a; return lhs; }
inline vec3 operator*  (vec3 lhs, const mat4 &a) { lhs *= a; return lhs; }
inline vec3 operator*  (vec3 lhs, const vec3 &rhs) { lhs *= rhs; return lhs; }
inline vec3 operator/  (vec3 lhs, const float a) { lhs /= a; return lhs; }
inline vec3 operator/  (vec3 lhs, const vec3 &rhs) { lhs /= rhs; return lhs; }

/*
void register_vec3lib(lua_State *L);
vec3 *push_vec3(lua_State* L, float x = 0, float y = 0, float z = 0);
vec3 *push_vec3(lua_State* L, const vec3 &v);*/
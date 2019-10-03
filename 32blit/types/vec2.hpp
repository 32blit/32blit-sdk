#pragma once

#include "math.h"

struct mat3;

struct vec2 {
  float x;
  float y;

  vec2(const vec2 &v) : x(v.x), y(v.y) {}
  vec2(const float x = 0, const float y = 0) : x(x), y(y) {}

  inline vec2& operator-= (const vec2 &a) { x -= a.x; y -= a.y; return *this; }
  inline vec2& operator+= (const vec2 &a) { x += a.x; y += a.y; return *this; }
  inline vec2& operator*= (const float a) { x *= a;   y *= a;   return *this; }
  inline vec2& operator*= (const mat3 &a) { this->transform(a); return *this; }
  inline vec2& operator/= (const float a) { x /= a;   y /= a;   return *this; }
  inline vec2& operator/= (const vec2 &a) { x /= a.x;   y /= a.y;   return *this; }

  void   transform(const mat3 &m);

  /**
   * Divide vector by its length
   */
  inline void   normalize() { float d = this->length(); x /= d; y /= d; }

  /**
   * Return length of vector
   * 
   * \return length
   */
  inline float  length() { return sqrtf(x * x + y * y); }

  /**
   * Return the cross product of two vectors
   * 
   * \param a `vec2` vector
   * \return `float` cross product
   */
  inline float   cross(const vec2 &a) { return x * a.y - y * a.x; }

  /**
   * Return the cross product of two vectors
   * 
   * \param a `vec2` vector
   * \return `float` cross product
   */
  inline float   cross(const vec2 *a) { return x * a->y - y * a->x; }

  /**
   * Return the dot product of two vectors
   * 
   * \param a `vec2` vector
   * \return `float` cross product
   */
  inline float  dot(const vec2 &a) { return (x * a.x) + (y * a.y); }

  /**
   * Return the dot product of two vectors
   * 
   * \param a `vec2` vector
   * \return `float` cross product
   */
  inline float  dot(const vec2 *a) { return (x * a->x) + (y * a->y); }

  /**
   * Rotate the vector
   * 
   * Performs an in-place rotation on the vector
   * 
   * \param a `float` angle of rotation (radians)
   */
  void   rotate(const float &a);

  /**
   * Get the angle between two vectors
   * 
   * \param o `vec2`
   */
  float   angle(vec2 o);

  static vec2   normalize(vec2 &v) { float d = v.length(); return vec2(v.x /= d, v.y /= d); }
};

inline vec2 operator-  (vec2 lhs, const vec2 &rhs) { lhs -= rhs; return lhs; }
inline vec2 operator-  (const vec2 &rhs) { return vec2(-rhs.x, -rhs.y); }
inline vec2 operator+  (vec2 lhs, const vec2 &rhs) { lhs += rhs; return lhs; }
inline vec2 operator*  (vec2 lhs, const float a) { lhs *= a; return lhs; }
inline vec2 operator*  (vec2 lhs, const mat3 &a) { lhs *= a; return lhs; }
inline vec2 operator/  (vec2 lhs, const float a) { lhs /= a; return lhs; }
inline vec2 operator/  (vec2 lhs, const vec2 &rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
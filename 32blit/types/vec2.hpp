#pragma once

#include <cmath>

namespace blit {

  struct Mat3;

  struct Vec2 {
    float x;
    float y;

    Vec2(const Vec2 &v) = default;
    explicit constexpr Vec2(const float x = 0, const float y = 0) : x(x), y(y) {}

    inline Vec2& operator-= (const Vec2 &a) { x -= a.x; y -= a.y; return *this; }
    inline Vec2& operator+= (const Vec2 &a) { x += a.x; y += a.y; return *this; }
    inline Vec2& operator*= (const float a) { x *= a;   y *= a;   return *this; }
    inline Vec2& operator*= (const Mat3 &a) { this->transform(a); return *this; }
    inline Vec2& operator*= (const Vec2 &a) { x *= a.x; y *= a.y; return *this; }
    inline Vec2& operator/= (const float a) { x /= a;   y /= a;   return *this; }
    inline Vec2& operator/= (const Vec2 &a) { x /= a.x;   y /= a.y;   return *this; }

    void   transform(const Mat3 &m);

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
    inline float   cross(const Vec2 &a) { return x * a.y - y * a.x; }

    /**
     * Return the cross product of two vectors
     *
     * \param a `vec2` vector
     * \return `float` cross product
     */
    inline float   cross(const Vec2 *a) { return x * a->y - y * a->x; }

    /**
     * Return the dot product of two vectors
     *
     * \param a `vec2` vector
     * \return `float` cross product
     */
    inline float  dot(const Vec2 &a) { return (x * a.x) + (y * a.y); }

    /**
     * Return the dot product of two vectors
     *
     * \param a `vec2` vector
     * \return `float` cross product
     */
    inline float  dot(const Vec2 *a) { return (x * a->x) + (y * a->y); }

    /**
     * Rotate the vector
     *
     * Performs an in-place rotation on the vector
     *
     * \param a `float` angle of rotation (radians)
     */
    void   rotate(float a);

    /**
     * Get the angle between two vectors
     *
     * \param o `vec2`
     */
    float   angle(Vec2 o);

    static Vec2   normalize(Vec2 &v) { float d = v.length(); return Vec2(v.x /= d, v.y /= d); }
  };

  inline Vec2 operator-  (Vec2 lhs, const Vec2 &rhs) { lhs -= rhs; return lhs; }
  inline Vec2 operator-  (const Vec2 &rhs) { return Vec2(-rhs.x, -rhs.y); }
  inline Vec2 operator+  (Vec2 lhs, const Vec2 &rhs) { lhs += rhs; return lhs; }
  inline Vec2 operator*  (Vec2 lhs, const float a) { lhs *= a; return lhs; }
  inline Vec2 operator*  (Vec2 lhs, const Mat3 &a) { lhs *= a; return lhs; }
  inline Vec2 operator*  (Vec2 lhs, const Vec2 &rhs) { lhs *= rhs; return lhs; }
  inline Vec2 operator/  (Vec2 lhs, const float a) { lhs /= a; return lhs; }
  inline Vec2 operator/  (Vec2 lhs, const Vec2 &rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }

}

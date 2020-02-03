/*! \file geometry.cpp
    \brief Geometry helper functions.

    These are some neat functions for geometry stuff!
*/
#include <string.h>
#include <float.h>

#include "../types/vec3.hpp"
#include "../types/vec2.hpp"

/**
 * Return the first point of intersection (if any) between a ray and a sphere.
 *
 * \param[in]  ray_origin         Origin of the ray
 * \param[in]  ray_vector         Vector of the ray
 * \param[in]  sphere_origin      Origin of the sphere
 * \param[in]  sphere_radius      Radius of the sphere
 * \param[out] point              Point of intersection
 * \param[out] distance           Distance to intersection
 * \param[out] normal             Normal of point on sphere surface where intersection occured
 *
 * \return `true` if intersection occurs
 */
bool ray_sphere_intersect(Vec3 ray_origin, Vec3 ray_vector, Vec3 sphere_origin, float sphere_radius, Vec3 *point = NULL, float *distance = NULL, Vec3 *normal = NULL) {
  Vec3 l = sphere_origin - ray_origin;
  float tca = l.dot(ray_vector);
  float d2 = l.dot(l) - tca * tca;

  if (d2 > (sphere_radius * sphere_radius)) {
    return false;
  }

  float thc = sqrtf(sphere_radius * sphere_radius - d2);
  float t0 = tca - thc;
  float t1 = tca + thc;

  if (t0 < 0) {
    t0 = t1;
  }

  if (t0 < 0) {
    return false;
  }

  // optional return values
  if (point) {
    *point = (ray_origin + ray_vector * t0);

    if (distance) {
      *distance = t0;
    }

    if (normal) {
      *normal = (*point - sphere_origin);
      normal->normalize();
    }
  }

  return true;
}

/**
 * Return the first point of intersection (if any) between a ray and a circle.
 *
 * \param[in]  ray_origin         Origin of the ray
 * \param[in]  ray_vector         Vector of the ray
 * \param[in]  circle_origin      Origin of the circle
 * \param[in]  circle_radius      Radius of the circle
 * \param[out] point              Point of intersection
 * \param[out] distance           Distance to intersection
 * \param[out] normal             Normal of point on circle circumference where intersection occured
 *
 * \return `true` if intersection occurs
 */
bool ray_circle_intersect(Vec2 ray_origin, Vec2 ray_vector, Vec2 circle_origin, float circle_radius, Vec2 *point = NULL, float *distance = NULL, Vec2 *normal = NULL) {  
  Vec2 l = ray_origin - circle_origin;

  float a = ray_vector.length() * ray_vector.length();
  float b = 2.0f * ray_vector.dot(l);
  float c = l.length() * l.length() - circle_radius * circle_radius;

  float q = b * b - 4.0f * a * c;

  if (q < 0.0f) {
    return false;
  }

  float g = 1.0f / (2.0f * a);
  q = g * sqrtf(q);
  b = (-b * g);

  if (b < 0.0f) {
    return false;
  }

  // optional return values
  if (point) {
    *point = ray_vector * (b - q) + ray_origin;

    if (distance) {
      *distance = (b - q);
    }

    if (normal) {
      *normal = (*point - circle_origin);
      normal->normalize();
    }
  }

  return true;
}

/**
 * Return the first point of intersection (if any) between a line and a circle.
 *
 * \param[in]  ray_origin         Origin of the ray
 * \param[in]  ray_vector         Vector of the ray
 * \param[in]  start              Start of the line
 * \param[in]  end                End of the line
 * \param[out] point              Point of intersection
 * \param[out] distance           Distance to intersection
 *
 * \return `true` if intersection occurs
 */
bool ray_line_intersect(Vec2 ray_origin, Vec2 ray_vector, Vec2 start, Vec2 end, Vec2 *point = NULL, float *distance = NULL) {
  Vec2 v1 = ray_origin - start;
  Vec2 v2 = end - start;
  Vec2 v3 = Vec2(-ray_vector.y, ray_vector.x);
  float d = v2.dot(v3);

  if (fabsf(d) < FLT_EPSILON) {
    return false;
  }

  float t1 = v2.cross(v1) / d;
  float t2 = v1.dot(v2) / d;

  if (t1 >= 0.0f && t2 >= 0.0f && t2 <= 1.0f) {
    *point = start + (v2 * t1);

    if (distance) {
      *distance = (*point - ray_origin).length();
    }
  }

  return false;
}
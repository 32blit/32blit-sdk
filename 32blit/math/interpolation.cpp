/*! \file interpolation.cpp
*/
#include "interpolation.hpp"

/**
 * TODO: Document function
 *
 * @param value
 * @param start
 * @param end
 * @param min
 * @param max
 * @return
 */
float lerp(float value, float start, float end, float min, float max) {
  value = value < start ? start : (value > end ? end : value);
  return ((value / (end - start)) * (max - min)) + min;
}

/**
 * TODO: Document function
 *
 * @param value
 * @param start
 * @param end
 * @return
 */
float lerp(float value, float start, float end) {
  return (value - start) / (end - start);
}

/**
 * TODO: Document function
 *
 * @param value
 * @param start
 * @param end
 * @param min
 * @param max
 * @return
 */
vec2 lerp(float value, float start, float end, vec2 min, vec2 max) {
  value = value < start ? start : (value > end ? end : value);
  return ((max - min) * (value / (end - start))) + min;
}

/**
 * TODO: Document function
 *
 * @param value
 * @param start
 * @param end
 */
vec2 lerp(float value, vec2 start, vec2 end) {
  return ((end - start) * value) + start;
}
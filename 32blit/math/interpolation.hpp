#pragma once

#include "../types/vec2.hpp"
namespace blit {
  float lerp(float value, float start, float end, float min, float max);
  float lerp(float value, float start, float end);
  Vec2 lerp(float value, float start, float end, Vec2 min, Vec2 max);
  Vec2 lerp(float value, Vec2 start, Vec2 end);
}
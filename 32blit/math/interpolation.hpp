#pragma once

#include "../types/vec2.hpp"

float lerp(float value, float start, float end, float min, float max);
float lerp(float value, float start, float end);
vec2 lerp(float value, float start, float end, vec2 min, vec2 max);
vec2 lerp(float value, vec2 start, vec2 end);

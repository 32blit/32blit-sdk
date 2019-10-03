#pragma once

#include <stdint.h>

#include "surface.hpp"
#include "../types/map.hpp"

namespace blit {

  void mode7(surface *dest, surface *tiles, MapLayer *layer, float fov, float angle, vec2 pos, float near, float far, rect viewport);
  vec2 world_to_screen(vec2 w, float fov, float angle, vec2 pos, float near, float far, rect viewport);
  float world_to_scale(vec2 w, float fov, float angle, vec2 pos, float near, float far, rect viewport);

}
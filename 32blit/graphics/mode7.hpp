#pragma once

#include <cstdint>

#include "surface.hpp"
#include "../types/map.hpp"

namespace blit {

  void mode7(Surface *dest, Surface *tiles, MapLayer *layer, float fov, float angle, Vec2 pos, float near, float far, Rect viewport);
  Vec2 world_to_screen(Vec2 w, float fov, float angle, Vec2 pos, float near, float far, Rect viewport);
  float world_to_scale(Vec2 w, float fov, float angle, Vec2 pos, float near, float far, Rect viewport);

}
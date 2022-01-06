#pragma once

#include "32blit.hpp"

using namespace blit;

extern uint32_t pixels_drawn;

void draw_face(Vec3* vertices, Vec3* normals, Vec2* texture_coordinates, Surface* texture, Vec3 light, Pen* color, int16_t *zbuffer, int16_t near, int16_t far);
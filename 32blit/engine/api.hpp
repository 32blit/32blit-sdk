#pragma once
#include <cstdint>
#include "input.hpp"

#include "../graphics/surface.hpp"
#include "../types/vec2.hpp"
#include "../types/vec3.hpp"

namespace blit {
  extern ButtonState &buttons;
  extern float &hack_left;
  extern float &hack_right;
  extern float &vibration;
  extern Vec2 &joystick;
  extern Vec3 &tilt;
  extern Pen &LED;
}
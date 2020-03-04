#pragma once

#include "../types/vec2.hpp"
#include "../types/vec3.hpp"
#include <cstdint>

namespace blit {
  enum Button : unsigned int {
    DPAD_LEFT = 1,
    DPAD_RIGHT = 2,
    DPAD_UP = 4,
    DPAD_DOWN = 8,
    A = 16,
    B = 32,
    X = 64,
    Y = 128,
    MENU = 256,
    HOME = 512,
    JOYSTICK = 1024
  };

  extern bool pressed(uint32_t button);

}

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

  struct ButtonState {
    ButtonState &operator=(uint32_t v) {
      uint32_t changed = state ^ v;

      pressed |= changed & v;
      released |= changed & state;

      state = v;

      return *this;
    }

    operator uint32_t() const {
      return state;
    }

    uint32_t state;
    uint32_t pressed, released; // state change since last update
  };

  extern bool pressed(uint32_t button);
}

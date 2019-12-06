/*! \file input.cpp
    \brief Input handlers
*/
#include "input.hpp"

namespace blit {
  
  uint32_t buttons;
  vec2 joystick;
  vec3 tilt;
  float hack_left;
  float hack_right;
  int battery;

  /**
   * Return pressed state of a button or buttons.
   *
   * \param button Bitmask for button(s) to read.
   * \return `true` for pressed, `false` for released.
   */
  bool pressed(uint32_t button) {
    return buttons & button;
  }

}
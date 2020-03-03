/*! \file input.cpp
    \brief Input handlers
*/
#include "input.hpp"

namespace blit {
  
  uint32_t buttons;
  Vec2 joystick;
  Vec3 tilt;
  float hack_left;
  float hack_right;

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
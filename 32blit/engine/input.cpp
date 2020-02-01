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
  float battery;
  uint8_t battery_vbus_status;
  uint8_t battery_charge_status;
  uint8_t battery_fault;

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
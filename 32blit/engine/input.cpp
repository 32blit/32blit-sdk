/*! \file input.cpp
    \brief Input handlers
*/
#include "input.hpp"
#include "api.hpp"

namespace blit {

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
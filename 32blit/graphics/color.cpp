
/*! \file color.cpp
    \brief Functions for managing colour.
*/
#include "color.hpp"

namespace blit {

  /**
   * Convert a colour from the Hue, Saturation, Value colour space to Red, Green, Blue, Alpha.
   * Alpha will always be set to 255.
   *
   * \param[in] h Hue from 0.0 to 1.0.
   * \param[in] s Saturation from 0.0 to 1.0.
   * \param[in] v Value from 0.0 to 1.0.
   * \return Pen colour.
   */
  Pen hsv_to_rgba(float h, float s, float v) {
    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
    default: //avoid warning
    case 0: return Pen(v, t, p);
    case 1: return Pen(q, v, p);
    case 2: return Pen(p, v, t);
    case 3: return Pen(p, q, v);
    case 4: return Pen(t, p, v);
    case 5: return Pen(v, p, q);
    }
  }

}

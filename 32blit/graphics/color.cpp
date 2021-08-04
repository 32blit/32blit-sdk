
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

    v = v * 255.0f;

    float sv = s * v;
    float fsv = f * sv;

    auto p = uint8_t(-sv + v);
    auto q = uint8_t(-fsv + v);
    auto t = uint8_t(fsv - sv + v);

    uint8_t bv = uint8_t(v);

    switch (i % 6) {
    default:
    case 0: return Pen(bv, t, p);
    case 1: return Pen(q, bv, p);
    case 2: return Pen(p, bv, t);
    case 3: return Pen(p, q, bv);
    case 4: return Pen(t, p, bv);
    case 5: return Pen(bv, p, q);
    }
  }

}

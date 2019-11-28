/*! \file mode7.cpp
    \brief Emulate mode7 graphics effects.

    Functions to emulate the mode7 graphics effect from classic consoles.
*/
#include "math.h"
#include <cmath>
#include <cfloat>

#include "../math/interpolation.hpp"
#include "mode7.hpp"

#ifndef M_PI
#define M_PI           3.14159265358979323846f  /* pi */
#endif

#include "../types/mat3.hpp"
#include "../graphics/font.hpp"

namespace blit {
 

  // TODO: Provide method to return scale for world coordinate

  /**
   * TODO: Document
   * 
   * \param[in] w
   * \param[in] fov
   * \param[in] angle
   * \param[in] pos
   * \param[in] near
   * \param[in] far
   * \param[in] viewport
   */
  vec2 world_to_screen(vec2 w, float fov, float angle, vec2 pos, float near, float far, rect viewport) {
    float hfov = fov / 2.0f;    

    vec2 v(w - pos);
    v.normalize();
    vec2 f(0, -1);
    f *= mat3::rotation(angle);

    float wd = (w - pos).length();
    float dot = f.dot(v);
    float det = f.x * v.y - f.y * v.x;
    float theta = atan2(det, dot);
    float costheta = cos(theta);
    float ctd = wd * costheta;

    float pd = ctd / cos(hfov);

    float theta_sign = std::copysign(1.0f, theta);
    
    float hsl = sqrt((pd * pd) - (ctd * ctd));
    float so = hsl + (sqrt((wd * wd) - (ctd * ctd)) * theta_sign);
    float r = so / (hsl * 2.0f);

    vec2 s(
      r * viewport.w,
      ((far - near) / (pd - near)) + viewport.y
    );
      
    return s;
  }

  /**
   * Convert a screen coordinate to a mode7 world-space coordinate.
   * 
   * \param[in] s vec2 describing the screen coordinate
   * \param[in] fov Current camera field-of-view
   * \param[in] angle Current camera z-angle in mode7 world-space
   * \param[in] pos Current camera position in mode7 world-space
   * \param[in] near Distance to nearest visible point
   * \param[in] far Distance to furthest visible point
   * \param[in] viewport
   */
  vec2 screen_to_world(vec2 s, float fov, float angle, vec2 pos, float near, float far, rect viewport) {
    vec2 forward(0, -1);
    forward *= mat3::rotation(angle);

    vec2 left = forward;
    left *= mat3::rotation((fov / 2.0f));

    vec2 right = forward;
    right *= mat3::rotation(-(fov / 2.0f));

    float distance = ((far - near) / float(s.y - viewport.y)) + near;
    
    vec2 swc = pos + (left * distance);
    vec2 ewc = pos + (right * distance);

    return lerp(s.x, viewport.x, viewport.x + viewport.w, swc, ewc);
  }


  // TODO: Add support for a default tile to draw outside of the bounds of the map and for the map to be repeated.

  /**
   * TODO: Document
   * 
   * \param[in] dest
   * \param[in] sprites
   * \param[in] layer
   * \param[in] fov
   * \param[in] angle
   * \param[in] pos
   * \param[in] near
   * \param[in] far
   * \param[in] viewport
   */
  void mode7(surface *dest, surface *sprites, MapLayer *layer, float fov, float angle, vec2 pos, float near, float far, rect viewport) {
    for (int y = viewport.y; y < viewport.y + viewport.h; y++) {
      vec2 swc = screen_to_world(vec2(viewport.x, y), fov, angle, pos, near, far, viewport);
      vec2 ewc = screen_to_world(vec2(viewport.x + viewport.w, y), fov, angle, pos, near, far, viewport);

      layer->mipmap_texture_span(
        dest,
        point(viewport.x, y),
        viewport.w,
        sprites,
        swc,
        ewc);
    }

    vec2 s = world_to_screen(vec2(400, 400), fov, angle, pos, near, far, viewport);
    dest->pen(rgba(255, 0, 255));
    dest->pixel(s);
  }
  
}

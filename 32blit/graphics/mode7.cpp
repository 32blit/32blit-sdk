/*! \file mode7.cpp
    \brief Emulate mode7 graphics effects.

    Functions to emulate the mode7 graphics effect from classic consoles.
*/
#include "math.h"
#include <cmath>
#include <cfloat>

#include "../math/interpolation.hpp"
#include "mode7.hpp"

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
  Vec2 world_to_screen(Vec2 w, float fov, float angle, Vec2 pos, float near, float far, Rect viewport) {
    float hfov = fov / 2.0f;    

    Vec2 v(w - pos);
    v.normalize();
    Vec2 f(0, -1);
    f *= Mat3::rotation(angle);

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

    Vec2 s(
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
  Vec2 screen_to_world(Vec2 s, float fov, float angle, Vec2 pos, float near, float far, Rect viewport) {
    Vec2 forward(0, -1);
    forward *= Mat3::rotation(angle);

    Vec2 left = forward;
    left *= Mat3::rotation((fov / 2.0f));

    Vec2 right = forward;
    right *= Mat3::rotation(-(fov / 2.0f));

    float distance = ((far - near) / float(s.y - viewport.y)) + near;
    
    Vec2 swc = pos + (left * distance);
    Vec2 ewc = pos + (right * distance);

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
  void mode7(Surface *dest, Surface *sprites, MapLayer *layer, float fov, float angle, Vec2 pos, float near, float far, Rect viewport) {
    for (int y = viewport.y; y < viewport.y + viewport.h; y++) {
      Vec2 swc = screen_to_world(Vec2(viewport.x, y), fov, angle, pos, near, far, viewport);
      Vec2 ewc = screen_to_world(Vec2(viewport.x + viewport.w, y), fov, angle, pos, near, far, viewport);

      layer->mipmap_texture_span(
        dest,
        Point(viewport.x, y),
        viewport.w,
        sprites,
        swc,
        ewc);
    }

    Vec2 s = world_to_screen(Vec2(400, 400), fov, angle, pos, near, far, viewport);
    dest->pen = Pen(255, 0, 255);
    dest->pixel(s);
  }
  
}

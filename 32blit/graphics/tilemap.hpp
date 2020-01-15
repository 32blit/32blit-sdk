#pragma once

#include <stdint.h>

#include "../32blit.hpp"
#include "../types/size.hpp"
#include "../types/point.hpp"
#include "../types/mat3.hpp"
#include "../graphics/sprite.hpp"

namespace blit {

  // A `tilemap` describes a grid of tiles with optional transforms
  struct tilemap {
    size          bounds;

    uint8_t      *tiles;
    uint8_t      *transforms;
    spritesheet  *sprites;
    mat3          transform = mat3::identity();

    enum {
      NONE = 0,           // draw nothing
      REPEAT = 1,         // infinite repeat
      DEFAULT_FILL = 2    // fill with default tile
    } repeat_mode;        // determines what to do when drawing outside of the layer bounds.
    uint8_t       default_tile_id;

    tilemap(uint8_t *tiles, uint8_t *transforms, size bounds, spritesheet *sprites);

    inline int32_t offset(const point &p); // __attribute__((always_inline));
    int32_t offset(const int16_t &x, const int16_t &y); // __attribute__((always_inline));
    uint8_t tile_at(const point &p); // __attribute__((always_inline));
    uint8_t transform_at(const point &p); // __attribute__((always_inline));

    void draw(surface *dest, rect viewport, std::function<mat3(uint8_t)> scanline_callback);

  //  void mipmap_texture_span(surface *dest, point s, uint16_t c, vec2 swc, vec2 ewc);
    void texture_span(surface *dest, point s, uint16_t c, vec2 swc, vec2 ewc);
  };

}
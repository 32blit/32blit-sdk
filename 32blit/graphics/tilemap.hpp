#pragma once

#include <cstdint>

#include "../32blit.hpp"
#include "../types/size.hpp"
#include "../types/point.hpp"
#include "../types/mat3.hpp"
#include "../graphics/sprite.hpp"

namespace blit {

  // A `tilemap` describes a grid of tiles with optional transforms
  struct TileMap {
    Size          bounds;

    uint8_t      *tiles;
    uint8_t      *transforms;
    Surface  *sprites;
    Mat3          transform = Mat3::identity();

    enum {
      NONE = 0,           // draw nothing
      REPEAT = 1,         // infinite repeat
      DEFAULT_FILL = 2    // fill with default tile
    } repeat_mode;        // determines what to do when drawing outside of the layer bounds.
    uint8_t       default_tile_id;

    int empty_tile_id = -1;

    TileMap(uint8_t *tiles, uint8_t *transforms, Size bounds, Surface *sprites);

    inline int32_t offset(const Point &p); // __attribute__((always_inline));
    int32_t offset(int16_t x, int16_t y); // __attribute__((always_inline));
    uint8_t tile_at(const Point &p); // __attribute__((always_inline));
    uint8_t transform_at(const Point &p); // __attribute__((always_inline));

    void draw(Surface *dest, Rect viewport, std::function<Mat3(uint8_t)> scanline_callback = nullptr);

  //  void mipmap_texture_span(surface *dest, point s, uint16_t c, vec2 swc, vec2 ewc);
    void texture_span(Surface *dest, Point s, unsigned int c, Vec2 swc, Vec2 ewc);
  };

}

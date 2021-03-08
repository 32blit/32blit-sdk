/*! \file tilemap.cpp
*/
#include <cstring>
#include "tilemap.hpp"

namespace blit {

  /**
   * Create a new tilemap.
   *
   * \param[in] tiles
   * \param[in] transforms
   * \param[in] bounds Map bounds, must be a power of two
   * \param[in] sprites
   */
  TileMap::TileMap(uint8_t *tiles, uint8_t *transforms, Size bounds, Surface *sprites) : bounds(bounds), tiles(tiles), transforms(transforms), sprites(sprites) {
    if (!transforms) {
      this->transforms = new uint8_t[bounds.w * bounds.h];
      std::memset(this->transforms, 0, bounds.w * bounds.h);
    }
  }

  /**
   * TODO: Document
   */
  int32_t TileMap::offset(const Point &p) {
    int32_t cx = ((uint32_t)p.x) & (bounds.w - 1);
    int32_t cy = ((uint32_t)p.y) & (bounds.h - 1);

    if ((p.x ^ cx) | (p.y ^ cy)) {
      if (repeat_mode == DEFAULT_FILL)
        return default_tile_id;

      if (repeat_mode == REPEAT)
        return cx + cy * bounds.w;

      return -1;
    }

    return cx + cy * bounds.w;
  }

  /**
   * TODO: Document
   *
   * \param[in] x
   * \param[in] y
   */
  int32_t TileMap::offset(int16_t x, int16_t y) {
    int32_t cx = ((uint32_t)x) & (bounds.w - 1);
    int32_t cy = ((uint32_t)y) & (bounds.h - 1);

    if ((x ^ cx) | (y ^ cy)) {
      if (repeat_mode == DEFAULT_FILL)
        return default_tile_id;

      if (repeat_mode == REPEAT)
        return cx + cy * bounds.w;

      return -1;
    }

    return cx + cy * bounds.w;
  }

  /**
   * Get flags for a specific tile.
   *
   * \param[in] p Point denoting the tile x/y position in the map.
   * \return Bitmask of flags for specified tile.
   */
  uint8_t TileMap::tile_at(const Point &p) {
    int32_t o = offset(p);

    if(o != -1)
      return tiles[o];

    return 0;
  }

  /**
   * Get transform for a specific tile.
   *
   * \param[in] p Point denoting the tile x/y position in the map.
   * \return Bitmask of transforms for specified tile.
   */
  uint8_t TileMap::transform_at(const Point &p) {
    int32_t o = offset(p);

    if (o != -1 && transforms)
      return transforms[o];

    return 0;
  }

  /**
   * Draw tilemap to a specified destination surface, with clipping.
   *
   * \param[in] dest Destination surface.
   * \param[in] viewport Clipping rectangle.
   * \param[in] scanline_callback Functon called on every scanline, accepts the scanline y position, should return a transformation matrix.
   */
  void TileMap::draw(Surface *dest, Rect viewport, std::function<Mat3(uint8_t)> scanline_callback) {
    //bool not_scaled = (from.w - to.w) | (from.h - to.h);

    viewport = dest->clip.intersection(viewport);

    for (uint16_t y = viewport.y; y < viewport.y + viewport.h; y++) {
      Vec2 swc(viewport.x, y);
      Vec2 ewc(viewport.x + viewport.w, y);

      if (scanline_callback) {
        Mat3 custom_transform = scanline_callback(y);
        swc *= custom_transform;
        ewc *= custom_transform;
      } else {
        swc *= transform;
        ewc *= transform;
      }

      texture_span(dest, Point(viewport.x, y), viewport.w, swc, ewc);
    }
  }

  /*
  void tilemap::mipmap_texture_span(surface *dest, point s, uint16_t c, vec2 swc, vec2 ewc) {
    // calculate the mipmap index to use for drawing
    float span_length = (ewc - swc).length();
    float mipmap = ((span_length / float(c)) / 2.0f);
    int16_t mipmap_index = floor(mipmap);
    uint8_t blend = (mipmap - floor(mipmap)) * 255;

    mipmap_index = mipmap_index >= (int)sprites->s.mipmaps.size() ? sprites->s.mipmaps.size() - 1 : mipmap_index;
    mipmap_index = mipmap_index < 0 ? 0 : mipmap_index;

    dest->alpha = 255;
    texture_span(dest, s, c, swc, ewc, mipmap_index);

    if (++mipmap_index < sprites->s.mipmaps.size()) {
      dest->alpha = blend;
      texture_span(dest, s, c, swc, ewc, mipmap_index);
    }
  }*/

  /**
   * TODO: Document
   *
   * \param[in] dest
   * \param[in] s
   * \param[in] c
   * \param[in] swc
   * \param[in] ewc
   */
  void TileMap::texture_span(Surface *dest, Point s, unsigned int c, Vec2 swc, Vec2 ewc) {
    Surface *src = sprites;

    static const int fix_shift = 16;

    Point wc(swc * (1 << fix_shift));
    Point dwc(((ewc - swc) / float(c)) * (1 << fix_shift));
    int32_t doff = dest->offset(s.x, s.y);

    do {
      int16_t wcx = wc.x >> fix_shift;
      int16_t wcy = wc.y >> fix_shift;

      int32_t toff = offset(wcx >> 3, wcy >> 3);

      if (toff != -1 && tiles[toff] != empty_tile_id) {
        uint8_t tile_id = tiles[toff];
        uint8_t transform = transforms[toff];

        // coordinate within sprite
        int u = wcx & 0b111;
        int v = wcy & 0b111;

        // if this tile has a transform then modify the uv coordinates
        if (transform) {
          v = (transform & 0b010) ? (7 - v) : v;
          u = (transform & 0b100) ? (7 - u) : u;
          if (transform & 0b001) { int tmp = u; u = v; v = tmp; }
        }

        // sprite sheet coordinates for top left corner of sprite
        u += (tile_id & 0b1111) * 8;
        v += (tile_id >> 4) * 8;

        // draw as many pixels as possible
        int count = 0;

        do {
          wc += dwc;
          c--;
          count++;
        } while(c && (wc.x >> fix_shift) == wcx && (wc.y >> fix_shift) == wcy);

        int soff = src->offset(u, v);
        dest->bbf(src, soff, dest, doff, count, 0);

        doff += count;

        continue;
      }

      // skip to next tile
      do {
        wc += dwc;
        doff++;
        c--;
      } while(c && (wc.x >> (fix_shift + 3)) == wcx >> 3 && (wc.y >> (fix_shift + 3)) == wcy >> 3);

    } while (c);
  }

}

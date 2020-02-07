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
   * \param[in] bounds
   * \param[in] sprites
   */
  TileMap::TileMap(uint8_t *tiles, uint8_t *transforms, Size bounds, SpriteSheet *sprites) : bounds(bounds), tiles(tiles), transforms(transforms), sprites(sprites) {
    if (!transforms) {
      this->transforms = new uint8_t[bounds.w * bounds.h];
      std::memset(this->transforms, 0, bounds.w * bounds.h);
    }
  }

  /**
   * TODO: Document
   */
  int32_t TileMap::offset(const Point &p) {
    int32_t cx = ((uint16_t)p.x) & (bounds.w - 1);
    int32_t cy = ((uint16_t)p.y) & (bounds.h - 1);
    
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
  int32_t TileMap::offset(const int16_t &x, const int16_t &y) {
    int32_t cx = ((uint16_t)x) & (bounds.w - 1);
    int32_t cy = ((uint16_t)y) & (bounds.h - 1);

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
  void TileMap::texture_span(Surface *dest, Point s, uint16_t c, Vec2 swc, Vec2 ewc) {
    /*BlendBlitFunc bbf = dest->bbf[static_cast<uint8_t>(sprites->format)];

    Surface *src = sprites;

    Vec2 wc = swc;
    Vec2 dwc = (ewc - swc) / float(c);
    int32_t doff = dest->offset(s.x, s.y);
    do {
      int16_t wcx = floor(wc.x);
      int16_t wcy = floor(wc.y);
     
      int32_t toff = offset(wcx >> 3, wcy >> 3);

      if (toff != -1) {
        uint8_t tile_id = tiles[toff];
        uint8_t transform = transforms[toff];
      
        // coordinate within sprite
        uint8_t u = wcx & 0b111;
        uint8_t v = wcy & 0b111;
      
        // if this tile has a transform then modify the uv coordinates
        if (transform) {  
          v = (transform & 0b010) ? (7 - v) : v;
          u = (transform & 0b100) ? (7 - u) : u;
          if (transform & 0b001) { uint8_t tmp = u; u = v; v = tmp; }
        }

        // sprite sheet coordinates for top left corner of sprite
        u += (tile_id & 0b1111) * 8;
        v += (tile_id >> 4) * 8;

        bbf(src, src->offset(u, v), dest, doff, 1, 1);
      }

      wc += dwc;
      doff++;
    } while (--c);*/
  }

}
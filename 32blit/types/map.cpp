/*! \file mat.cpp
*/
#include <algorithm>
#include "map.hpp"

using namespace blit;

void Map::add_layer(std::string name, std::vector<uint8_t> tiles) {
  MapLayer &layer = layers[name];
  layer.map = this;
  layer.name = name;
  layer.tiles = tiles;
}


Map::Map(Rect bounds) : bounds(bounds) {
  flags.resize(bounds.w * bounds.h);
}

int32_t Map::tile_index(Point p) {
  return this->bounds.contains(p) ? p.x + p.y * this->bounds.w : -1;
}

void MapLayer::add_flags(uint8_t t, uint8_t f) {
  for (uint32_t i = 0; i < this->tiles.size(); i++) {
    if (this->tiles[i] == t)
      map->flags[i] |= f;
  }
}

void MapLayer::mipmap_texture_span(Surface *dest, Point s, uint16_t c, Surface *sprites, Vec2 swc, Vec2 ewc) {
  // calculate the mipmap index to use for drawing
  float span_length = (ewc - swc).length();
  float mipmap = ((span_length / float(c)) / 2.0f);
  uint16_t mipmap_index = floor(mipmap);
  uint8_t blend = (mipmap - floor(mipmap)) * 255;

  mipmap_index = mipmap_index >= (int)sprites->mipmaps.size() ? sprites->mipmaps.size() - 1 : mipmap_index;
  mipmap_index = mipmap_index < 0 ? 0 : mipmap_index;

  dest->alpha = 255;
  texture_span(dest, s, c, sprites->mipmaps[mipmap_index], swc, ewc, mipmap_index);
  
  if (++mipmap_index < sprites->mipmaps.size()) {    
    dest->alpha = blend;
    texture_span(dest, s, c, sprites->mipmaps[mipmap_index], swc, ewc, mipmap_index);
  }
}

void MapLayer::texture_span(Surface *dest, Point s, uint16_t c, Surface *sprites, Vec2 swc, Vec2 ewc, uint8_t mipmap_index = 0) {
  /*BlendBlitFunc bbf = dest->bbf[static_cast<uint8_t>(sprites->format)];

  int world_size = map->bounds.w * 8;
  int tile_size = 8 >> mipmap_index;
  
  Vec2 wc = swc;
  Vec2 dwc = (ewc - swc) / float(c);
  for (int x = s.x; x < s.x + c; x++) {   
    if (wc.x >= 0 && wc.x < world_size && wc.y >= 0 && wc.y < world_size) {
      Point pt(wc.x / 8, wc.y / 8); // tile point

      int16_t tile_id = tile_at(pt) - 1;

      if (tile_id != -1) {
        Point sp(
          (tile_id & 0b1111) * tile_size,
          (tile_id / 16) * tile_size
        ); // sprite sheet coordinates

        Point uv(
          (uint8_t(wc.x) & 0b111) >> mipmap_index,
          (uint8_t(wc.y) & 0b111) >> mipmap_index
        ); // texture coordinates
        
        // apply uv transform for tile
        uint8_t transform = transform_at(pt);
        if (transform & 0b010) { uv.y = (tile_size - 1) - uv.y; }
        if (transform & 0b100) { uv.x = (tile_size - 1) - uv.x; }
        if (transform & 0b001) { uint8_t tmp = uv.x; uv.x = uv.y; uv.y = tmp; }

        Point t = sp + uv;
        
        bbf(sprites, sprites->offset(t), dest, dest->offset(x, s.y), 1, 1);
      }
    }

    wc += dwc;
  }  */
}

void MapLayer::add_flags(std::vector<uint8_t> ts, uint8_t f) {
  for (auto t : ts) {
    this->add_flags(t, f);
  }
}

uint8_t MapLayer::tile_at(Point p) {
  int32_t ti = map->tile_index(p);
  return ti == -1 ? 0 : tiles[ti];
}

uint8_t MapLayer::transform_at(Point p) {
  int32_t ti = map->tile_index(p);
  return ti == -1 ? 0 : transforms[ti];
}

uint8_t Map::get_flags(Point p) {
  int32_t ti = tile_index(p);
  return ti == -1 ? 0 : this->flags[ti];
}

bool Map::has_flag(Point p, uint8_t f) {
  return this->get_flags(p) & f;
}

void Map::tiles_in_rect(Rect vp, std::function<void(Point)> f) {
  int minx = vp.x / 8;
  int maxx = ((vp.x + vp.w) / 8);
  int miny = vp.y / 8;
  int maxy = ((vp.y + vp.h) / 8);

  minx = minx < 0 ? 0 : minx;
  miny = miny < 0 ? 0 : miny;

  maxx = maxx > bounds.w - 1 ? bounds.w - 1 : maxx;
  maxy = maxy > bounds.h - 1 ? bounds.h - 1 : maxy;

  Point pt;
  for (pt.y = miny; pt.y <= maxy; pt.y++) {
    for (pt.x = minx; pt.x <= maxx; pt.x++) {
      f(pt);
    }
  }
}

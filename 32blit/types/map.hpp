#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>

#include "../types/rect.hpp"
#include "../graphics/surface.hpp"

using namespace blit;

struct Map;

struct MapLayer {
  Map *map;

  std::string name;
  std::vector<uint8_t> tiles;  
  std::vector<uint8_t> transforms;

  void add_flags(uint8_t t, uint8_t f);
  void add_flags(std::vector<uint8_t> t, uint8_t f);  

  uint8_t tile_at(point p);
  uint8_t transform_at(point p);

  void mipmap_texture_span(surface *dest, point s, uint16_t c, surface *sprites, vec2 swc, vec2 ewc);
  void texture_span(surface *dest, point s, uint16_t c, surface *sprites, vec2 swc, vec2 ewc, uint8_t mipmap_index);
};

struct Map {
  rect bounds;

  std::map<std::string, MapLayer> layers;
  std::vector<uint8_t> flags;

  Map(rect bounds);

  void add_layer(std::string name, std::vector<uint8_t> tiles);  
  int32_t tile_index(point p);

  uint8_t get_flags(point p);
  bool has_flag(point p, uint8_t f);

  void tiles_in_rect(rect r, std::function<void(point)> f);

};
#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>

#include "../types/rect.hpp"
#include "../graphics/surface.hpp"

namespace blit {
  struct Map;

  struct MapLayer {
    Map *map;

    std::string name;
    std::vector<uint8_t> tiles;
    std::vector<uint8_t> transforms;

    void add_flags(uint8_t t, uint8_t f);
    void add_flags(std::vector<uint8_t> t, uint8_t f);

    uint8_t tile_at(blit::Point p);
    uint8_t transform_at(blit::Point p);

    void mipmap_texture_span(blit::Surface *dest, blit::Point s, uint16_t c, blit::Surface *sprites, Vec2 swc, Vec2 ewc);
    void texture_span(blit::Surface *dest, blit::Point s, uint16_t c, blit::Surface *sprites, Vec2 swc, Vec2 ewc, uint8_t mipmap_index);
  };

  struct Map {
    blit::Rect bounds;

    std::map<std::string, MapLayer> layers;
    std::vector<uint8_t> flags;

    Map(blit::Rect bounds);

    void add_layer(std::string name, std::vector<uint8_t> tiles);
    int32_t tile_index(blit::Point p);

    uint8_t get_flags(blit::Point p);
    bool has_flag(blit::Point p, uint8_t f);

    void tiles_in_rect(blit::Rect r, std::function<void(blit::Point)> f);

  };
}
/*! \file tilemap.cpp
*/
#include <cstring>
#include "tilemap.hpp"

namespace blit {
  // TMX load helper
  static bool load_tmx_layer_data(const TMX *map_struct, File &file, bool require_pot, int layer, int &flags, uint8_t *&tile_data, uint8_t *&transform_data) {
    if(memcmp(map_struct, "MTMX", 4) != 0 || map_struct->header_length != sizeof(TMX))
      return false;

    // check power of two bounds if required
    if(require_pot && ((map_struct->width & (map_struct->width - 1)) || (map_struct->height & (map_struct->height - 1))))
      return false;

    bool is_16bit = map_struct->flags & TMX_16BIT;

    auto layer_size = map_struct->width * map_struct->height;

    if(is_16bit) {
      layer_size *= 2;
      flags |= TILES_16BIT;
    }

    if(flags & COPY_TILES) {
      tile_data = new uint8_t[layer_size];
      file.read(map_struct->header_length + layer_size * layer, layer_size, (char *)tile_data);
    } else if(file.get_ptr()) {
      tile_data = const_cast<uint8_t *>(file.get_ptr() + map_struct->header_length + layer_size * layer);
    }

    auto transform_base = map_struct->header_length + layer_size * map_struct->layers;
    auto transform_layer_size = map_struct->width * map_struct->height;

    if(flags & COPY_TRANSFORMS) {
      transform_data = new uint8_t[transform_layer_size]();

      if(map_struct->flags & TMX_TRANSFORMS)
        file.read(transform_base + transform_layer_size * layer, transform_layer_size, (char *)transform_data);
    } else if((map_struct->flags & TMX_TRANSFORMS) && file.get_ptr()) {
      transform_data = const_cast<uint8_t *>(file.get_ptr() + transform_base + transform_layer_size * layer);
    }

    return true;
  }

  static uint32_t calc_tmx_size(const TMX *map_struct) {
    // the TMX header contains many things, but a total size is not one of them
    int tile_id_size = (map_struct->flags & TMX_16BIT) ? 2 : 1;
    int transform_size = (map_struct->flags & TMX_TRANSFORMS) ? 1 : 0;

    return map_struct->header_length
         + map_struct->layers * map_struct->width * map_struct->height * tile_id_size
         + map_struct->layers * map_struct->width * map_struct->height * transform_size;
  }

  /**
   * Create a new tile layer.
   *
   * \param[in] tiles
   * \param[in] transforms
   * \param[in] bounds Map bounds
   * \param[in] sprites
   */
  TileLayer::TileLayer(uint8_t *tiles, uint8_t *transforms, Size bounds, Surface *sprites) : bounds(bounds), tiles(tiles), transforms(transforms), sprites(sprites) {
  }

  TileLayer::~TileLayer() {
    if(load_flags & MapLoadFlags::COPY_TILES)
      delete[] tiles;

    if(load_flags & MapLoadFlags::COPY_TRANSFORMS)
      delete[] transforms;
  }

  /**
   * TODO: Document
   *
   * \param[in] x
   * \param[in] y
   */
  int32_t TileLayer::offset(int16_t x, int16_t y) {
    if (x < 0 || y < 0 || x >= bounds.w || y >= bounds.h) {
      if (repeat_mode == DEFAULT_FILL)
        return default_tile_id;

      int32_t cx = x % bounds.w + (x < 0 ? bounds.w : 0);
      int32_t cy = y % bounds.h + (y < 0 ? bounds.h : 0);

      if (repeat_mode == REPEAT)
        return cx + cy * bounds.w;

      if(repeat_mode == CLAMP_TO_EDGE) {
        if(x != cx)
          cx = x < 0 ? 0 : bounds.w - 1;
        if(y != cy)
          cy = y < 0 ? 0 : bounds.h - 1;

        return cx + cy * bounds.w;
      }

      return -1;
    }

    return x + y * bounds.w;
  }

  /**
   * Get flags for a specific tile.
   *
   * \param[in] p Point denoting the tile x/y position in the map.
   * \return Bitmask of flags for specified tile.
   */
  uint16_t TileLayer::tile_at(const Point &p) {
    int32_t o = offset(p.x, p.y);

    if(o != -1) {
      if(load_flags & TILES_16BIT)
        return ((uint16_t *)tiles)[o];
      else
        return tiles[o];
    }

    return 0;
  }

  /**
   * Get transform for a specific tile.
   *
   * \param[in] p Point denoting the tile x/y position in the map.
   * \return Bitmask of transforms for specified tile.
   */
  uint8_t TileLayer::transform_at(const Point &p) {
    int32_t o = offset(p.x, p.y);

    if (o != -1 && transforms)
      return transforms[o];

    return 0;
  }

 /**
   * Create a new tilemap
   *
   * \param[in] tiles
   * \param[in] transforms
   * \param[in] bounds Map bounds
   * \param[in] sprites
   */
  SimpleTileLayer::SimpleTileLayer(uint8_t *tiles, uint8_t *transforms, Size bounds, Surface *sprites) : TileLayer(tiles, transforms, bounds, sprites){
  }

  SimpleTileLayer *SimpleTileLayer::load_tmx(const uint8_t *asset, Surface *sprites, int layer, int flags) {
    File map_file(asset, calc_tmx_size((TMX*)asset));

    return load_tmx(map_file, sprites, layer, flags);
  }

  SimpleTileLayer *SimpleTileLayer::load_tmx(File &file, Surface *sprites, int layer, int flags) {
    TMX map_struct;

    file.read(0, sizeof(map_struct), (char *)&map_struct);

    uint8_t *tile_data = nullptr;
    uint8_t *transform_data = nullptr;

    if(!load_tmx_layer_data(&map_struct, file, false, layer, flags, tile_data, transform_data))
      return nullptr;

    auto ret = new SimpleTileLayer(tile_data, transform_data, Size(map_struct.width, map_struct.height), sprites);
    ret->empty_tile_id = map_struct.empty_tile;
    ret->load_flags = flags;

    return ret;
  }

  /**
   * Draw tilemap to a specified destination surface, with clipping.
   *
   * \param[in] dest Destination surface.
   * \param[in] viewport Clipping rectangle.
   */
  void SimpleTileLayer::draw(Surface *dest, Rect viewport) {
    viewport = dest->clip.intersection(viewport);
    Rect old_clip = dest->clip;
    dest->clip = viewport;

    Point scroll_offset(transform.v02, transform.v12);

    Point start = (viewport.tl() + scroll_offset) / 8;
    Point end = (viewport.br() + scroll_offset) / 8;

    for(int y = start.y - 1; y <= end.y; y++) {
      for(int x = start.x - 1; x <= end.x; x++) {
        auto tile_offset = offset(x, y);

        // out-of-bounds
        if(tile_offset == -1)
          continue;

        int tile_id;

        if(load_flags & TILES_16BIT)
          tile_id = ((uint16_t *)tiles)[tile_offset];
        else
          tile_id = tiles[tile_offset];

        // empty tile
        if(tile_id == empty_tile_id)
          continue;

        int transform = transforms ? transforms[tile_offset] : 0;
        int sprite_transform = 0;

        // tilemap/sprite transform bits don't match
        if(transform & 0b001)
          sprite_transform |= SpriteTransform::XYSWAP;
        if(transform & 0b010)
          sprite_transform |= SpriteTransform::VERTICAL;
        if(transform & 0b100)
          sprite_transform |= SpriteTransform::HORIZONTAL;

        Rect src_rect = sprites->sprite_bounds(tile_id);
        dest->blit(sprites, src_rect, {x * 8 - scroll_offset.x, y * 8 - scroll_offset.y}, sprite_transform);
      }
    }

    dest->clip = old_clip;
  }

  /**
   * Create a new tilemap
   *
   * \param[in] tiles
   * \param[in] transforms
   * \param[in] bounds Map bounds, must be a power of two
   * \param[in] sprites
   */
  TransformedTileLayer::TransformedTileLayer(uint8_t *tiles, uint8_t *transforms, Size bounds, Surface *sprites) : TileLayer(tiles, transforms, bounds, sprites){
  }

  TransformedTileLayer *TransformedTileLayer::load_tmx(const uint8_t *asset, Surface *sprites, int layer, int flags) {
    File map_file(asset, calc_tmx_size((TMX*)asset));

    return load_tmx(map_file, sprites, layer, flags);
  }

  TransformedTileLayer *TransformedTileLayer::load_tmx(File &file, Surface *sprites, int layer, int flags) {
    TMX map_struct;

    file.read(0, sizeof(map_struct), (char *)&map_struct);

    uint8_t *tile_data = nullptr;
    uint8_t *transform_data = nullptr;

    if(!load_tmx_layer_data(&map_struct, file, true, layer, flags, tile_data, transform_data))
      return nullptr;

    auto ret = new TransformedTileLayer(tile_data, transform_data, Size(map_struct.width, map_struct.height), sprites);
    ret->empty_tile_id = map_struct.empty_tile;
    ret->load_flags = flags;

    return ret;
  }

  /**
   * Draw tilemap to a specified destination surface, with clipping.
   *
   * \param[in] dest Destination surface.
   * \param[in] viewport Clipping rectangle.
   * \param[in] scanline_callback Functon called on every scanline, accepts the scanline y position, should return a transformation matrix.
   */
  void TransformedTileLayer::draw(Surface *dest, Rect viewport, std::function<Mat3(uint8_t)> scanline_callback) {
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

  void TransformedTileLayer::mipmap_texture_span(Surface *dest, Point s, uint16_t c, Vec2 swc, Vec2 ewc) {
    // calculate the mipmap index to use for drawing
    float span_length = (ewc - swc).length();
    float mipmap = ((span_length / float(c)) / 2.0f);
    uint16_t mipmap_index = floorf(mipmap);
    uint8_t blend = (mipmap - floorf(mipmap)) * 255;

    mipmap_index = mipmap_index >= sprites->mipmaps.size() ? sprites->mipmaps.size() - 1 : mipmap_index;

    dest->alpha = 255;
    texture_span(dest, s, c, swc, ewc, sprites->mipmaps[mipmap_index], mipmap_index);

    if (++mipmap_index < sprites->mipmaps.size() && blend) {
      dest->alpha = blend;
      texture_span(dest, s, c, swc, ewc, sprites->mipmaps[mipmap_index], mipmap_index);
    }
  }

  /**
   * TODO: Document
   *
   * \param[in] dest
   * \param[in] s
   * \param[in] c
   * \param[in] swc
   * \param[in] ewc
   */
  void TransformedTileLayer::texture_span(Surface *dest, Point s, unsigned int c, Vec2 swc, Vec2 ewc, Surface *src, unsigned int mipmap_index) {
    if(load_flags & TILES_16BIT)
      do_texture_span<uint16_t>(dest, s, c, swc, ewc, src, mipmap_index);
    else
      do_texture_span<uint8_t>(dest, s, c, swc, ewc, src, mipmap_index);
  }

  template<class tile_id_type>
  void TransformedTileLayer::do_texture_span(Surface *dest, Point s, unsigned int c, Vec2 swc, Vec2 ewc, Surface *src, unsigned int mipmap_index) {
    if(!src)
      src = sprites;

    static const int fix_shift = 16;

    Point wc(swc * (1 << fix_shift));
    Point dwc(((ewc - swc) / float(c)) * (1 << fix_shift));
    int32_t doff = dest->offset(s.x, s.y);

    do {
      int16_t wcx = wc.x >> fix_shift;
      int16_t wcy = wc.y >> fix_shift;

      int32_t toff = fast_offset(wcx >> 3, wcy >> 3);

      if (toff != -1 && ((tile_id_type *)tiles)[toff] != empty_tile_id) {
        uint8_t tile_id = ((tile_id_type *)tiles)[toff];
        uint8_t transform = transforms ? transforms[toff] : 0;

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

        auto pen = src->get_pixel({u >> mipmap_index, v >> mipmap_index});
        dest->pbf(&pen, dest, doff, count);

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

  int32_t TransformedTileLayer::fast_offset(int16_t x, int16_t y) {
    // this is the reason for the power of two size restriction
    // (doing divides every pixel would hurt performance quite a bit)
    int32_t cx = ((uint32_t)x) & (bounds.w - 1);
    int32_t cy = ((uint32_t)y) & (bounds.h - 1);

    if ((x ^ cx) | (y ^ cy)) {
      if (repeat_mode == DEFAULT_FILL)
        return default_tile_id;

      if (repeat_mode == REPEAT)
        return cx + cy * bounds.w;

      if(repeat_mode == CLAMP_TO_EDGE) {
        if(x != cx)
          cx = x < 0 ? 0 : bounds.w - 1;
        if(y != cy)
          cy = y < 0 ? 0 : bounds.h - 1;

        return cx + cy * bounds.w;
      }

      return -1;
    }

    return cx + cy * bounds.w;
  }

  /// Create an empty map
  TiledMap::TiledMap(Size bounds, unsigned num_layers, Surface *sprites, int flags) : num_layers(num_layers) {
    layers = new TileLayer *[num_layers];
    for(unsigned i = 0; i < num_layers; i++) {
      if(flags & LAYER_TRANSFORMS)
        layers[i] = new TransformedTileLayer(nullptr, nullptr, bounds, sprites);
      else
        layers[i] = new SimpleTileLayer(nullptr, nullptr, bounds, sprites);

      layers[i]->load_flags = flags;
    }
  }

  /// Create from a map asset generated with `output_struct=true`
  TiledMap::TiledMap(const uint8_t *asset, Surface *sprites, int flags) {
    auto map_struct = reinterpret_cast<const TMX *>(asset);

    // header check
    if(memcmp(map_struct, "MTMX", 4) != 0 || map_struct->header_length != sizeof(TMX))
      return;

    num_layers = map_struct->layers;

    // load each layer
    layers = new TileLayer *[num_layers];

    for(unsigned i = 0; i < num_layers; i++) {
      if(flags & LAYER_TRANSFORMS)
        layers[i] = TransformedTileLayer::load_tmx(asset, sprites, i, flags);
      else
        layers[i] = SimpleTileLayer::load_tmx(asset, sprites, i, flags);
    }
  }

  /// Create from a map file generated with `output_struct=true`
  TiledMap::TiledMap(const std::string &filename, Surface *sprites, int flags) {
    File map_file(filename);

    if(!map_file.is_open())
      return;

    TMX map_struct;

    map_file.read(0, sizeof(map_struct), (char *)&map_struct);

    // header check
    if(memcmp(map_struct.head, "MTMX", 4) != 0 || map_struct.header_length != sizeof(TMX))
      return;

    num_layers = map_struct.layers;

    // load each layer
    layers = new TileLayer *[num_layers];

    for(unsigned i = 0; i < num_layers; i++) {
      if(flags & LAYER_TRANSFORMS)
        layers[i] = TransformedTileLayer::load_tmx(map_file, sprites, i, flags);
      else
        layers[i] = SimpleTileLayer::load_tmx(map_file, sprites, i, flags);
    }
  }

  TiledMap::~TiledMap() {
    for(unsigned i = 0; i < num_layers; i++)
      delete layers[i];

    delete[] layers;
  }

  /// Draw map to a viewport in `dest`
  void TiledMap::draw(Surface *dest, Rect viewport) {
    for(unsigned i = 0; i < num_layers; i++) {
      if(layers[i])
        layers[i]->draw(dest, viewport);
    }
  }

  void TiledMap::draw(Surface *dest, Rect viewport, std::function<Mat3(uint8_t)> scanline_callback) {
    for(unsigned i = 0; i < num_layers; i++) {
      if(layers[i] && (layers[i]->load_flags & LAYER_TRANSFORMS))
        static_cast<TransformedTileLayer *>(layers[i])->draw(dest, viewport, scanline_callback);
    }
  }

  /// Get a layer of the map, or `nullptr` if the index doesn't exist
  TileLayer* TiledMap::get_layer(unsigned index) {
    if(index < num_layers)
      return layers[index];

    return nullptr;
  }

  /// Get map bounds
  Size TiledMap::get_bounds() const {
    if(!num_layers || !layers[0])
      return {0, 0};

    // all layers have the same bounds
    return layers[0]->bounds;
  }

  void TiledMap::set_scroll_position(Point scroll_position) {
    set_transform(Mat3::translation(Vec2(scroll_position)));
  }

  void TiledMap::set_scroll_position(unsigned layer, Point scroll_position) {
    set_transform(layer, Mat3::translation(Vec2(scroll_position)));
  }

  void TiledMap::set_transform(Mat3 transform) {
    for(unsigned i = 0; i < num_layers; i++) {
      if(layers[i])
        layers[i]->transform = transform;
    }
  }

  void TiledMap::set_transform(unsigned layer, Mat3 transform) {
    if(layer < num_layers && layers[layer])
      layers[layer]->transform = transform;
  }
}

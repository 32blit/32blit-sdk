#pragma once
#include <functional>
#include <memory>
#include <vector>
#include <array>
#include <stdint.h>

#ifdef WIN32 
#define __attribute__(A)
#endif

#include "../types/rect.hpp"
#include "../types/size.hpp"
#include "../types/pixel_format.hpp"
#include "../graphics/blend.hpp"

namespace blit {

  struct spritesheet;
  struct sprite_p;

#pragma pack(push, 1)
  struct packed_image {
    uint8_t type[8];
    uint16_t byte_count;
    uint16_t width;
    uint16_t height;
    uint16_t cols;
    uint16_t rows;
    uint8_t format;
    uint8_t palette_entry_count;
  };
#pragma pack(pop)

  enum text_align {
    left          = 0b0000,
    center_h      = 0b0100,
    right         = 0b1000,
    top           = 0b0000,
    center_v      = 0b0001,
    bottom        = 0b0010,

    // combinations of above
    top_left      = top      | left,
    center_left   = center_v | left,
    bottom_left   = bottom   | left,
    top_center    = top      | center_h,
    center_center = center_v | center_h,
    bottom_center = bottom   | center_h,
    top_right     = top      | right,
    center_right  = center_v | right,
    bottom_right  = bottom   | right,
  };

  struct surface {

    uint8_t                        *data;                     // pointer to pixel data (for `rgba` format has pre-multiplied alpha)
    size                            bounds;                   // size of surface in pixels
    
    rect                            clip;                     // clip rectangle

    rgba                            _pen;                     // current pen
    uint8_t                         alpha = 255;              // global alpha for drawing operations

    pixel_format                    format;                   // surface pixel format
    uint8_t                         stride;                   // bytes per pixel
    uint16_t                        row_stride;               // bytes per row

    surface                        *mask = nullptr;           // mask pointer

    spritesheet                    *sprites = nullptr;        // active spritesheet

    std::vector<rgba>               palette;                  // palette entries (for paletted images)
    uint8_t                         transparent_index = 0;    // index of transparent colour (for paletted surfaces)

    // blend functions
    blit::blend_span_func           bf;
    std::array<blend_blit_func, 5>  bbf;
    std::vector<surface *>          mipmaps;

  private:
    void init();
    void load_from_packed(const packed_image *image);

  public:    
    surface(uint8_t *data, const pixel_format &format, const size &bounds);
    surface(uint8_t *data, const pixel_format &format, const packed_image *image);

    surface *load(const packed_image *image);

    // helpers to retrieve pointer to pixel
    __attribute__((always_inline)) uint8_t* ptr(const rect &r)   { return data + r.x * stride + r.y * row_stride; }
    __attribute__((always_inline)) uint8_t* ptr(const point &p)  { return data + p.x * stride + p.y * row_stride; }
    __attribute__((always_inline)) uint8_t* ptr(const int32_t &x, const int32_t &y) { return data + x * stride + y * row_stride; }

    __attribute__((always_inline)) uint32_t offset(const rect &r) { return r.x + r.y * bounds.w; }
    __attribute__((always_inline)) uint32_t offset(const point &p) { return p.x + p.y * bounds.w; }
    __attribute__((always_inline)) uint32_t offset(const int32_t &x, const int32_t &y) { return x + y * bounds.w; }    

    void generate_mipmaps(uint8_t depth);

    void pen(rgba v);

    void clear();
    void pixel(const point &p);
    void _pixel(const point &p);
    void _pixel(const uint32_t &o);
    void v_span(point p, int16_t c); 
    void h_span(point p, int16_t c); 
    void rectangle(const rect &r);
    void circle(const point &c, int32_t r);

    void line(const point &p1, const point &p2);
    void triangle(point p1, point p2, point p3);
    void polygon(std::vector<point> p);

    void text(std::string message, const uint8_t *font, const rect &r, bool variable = true, text_align align = text_align::top_left);
    void text(std::string message, const uint8_t *font, const point &p, bool variable = true, text_align align = text_align::top_left);
    size measure_text(std::string message, const uint8_t *font, bool variable = true);
    std::string wrap_text(std::string message, int32_t width, const uint8_t *font, bool variable = true);

    /*void outline_circle(const point &c, int32_t r);
    
    */ 
    void blit(surface *src, rect r, point p, bool hflip = false);
    void stretch_blit(surface *src, rect sr, rect dr);
    void stretch_blit_vspan(surface *src, point uv, uint16_t sc, point p, int16_t dc);

    void custom_blend(surface *src, rect r, point p, std::function<void(uint8_t *psrc, uint8_t *pdest, int16_t c)> f);
    void custom_modify(rect r, std::function<void(uint8_t *p, int16_t c)> f);
    void watermark();


    // origin           (optional - default 0, 0)
    // span repeat x/y  (optional - default 1, 1)
    // scale            (optional - default 1)

    //void sprite(spritesheet &ss, uint16_t index, point pos, size span = size(1, 1), point origin = point(0, 0), float scale = 1.0f);
    //void sprite(spritesheet &ss, point sprite, point position, sprite_p &properties);

    
    void blit_sprite(const rect &src, const point &p, const uint8_t &t = 0);
    void stretch_blit_sprite(const rect &src, const rect &r, const uint8_t &t = 0);

    void sprite(const rect &sprite, const point &position, const uint8_t &transform = 0);
    void sprite(const point &sprite, const point &position, const uint8_t &transform = 0);
    void sprite(const uint16_t &sprite, const point &position, const uint8_t &transform = 0);

    void sprite(const rect &sprite, const point &position, const point &origin, const uint8_t &transform = 0);
    void sprite(const point &sprite, const point &position, const point &origin, const uint8_t &transform = 0);
    void sprite(const uint16_t &sprite, const point &position, const point &origin, const uint8_t &transform = 0);

    void sprite(const rect &sprite, const point &position, const point &origin, const vec2 &scale, const uint8_t &transform = 0);
    void sprite(const point &sprite, const point &position, const point &origin, const vec2 &scale, const uint8_t &transform = 0);
    void sprite(const uint16_t &sprite, const point &position, const point &origin, const vec2 &scale, const uint8_t &transform = 0);
    
    void sprite(const rect &sprite, const point &position, const point &origin, const float &scale, const uint8_t &transform = 0);
    void sprite(const point &sprite, const point &position, const point &origin, const float &scale, const uint8_t &transform = 0);
    void sprite(const uint16_t &sprite, const point &position, const point &origin, const float &scale, const uint8_t &transform = 0);

    //extern void texture_triangle(int32_t x1, int32_t y1, int32_t u1, int32_t v1, int32_t x2, int32_t y2, int32_t u2, int32_t v2, int32_t x3, int32_t y3, int32_t u3, int32_t v3);
    
    /*
      blitting methods
    */
    //extern void blit(rgba *src, int16_t sx, int16_t sy, int16_t sw, int16_t w, int16_t h, int16_t dx, int16_t dy, bool hflip);
  /*  void blit(surface *src, rect from, const point &to, uint8_t flip = 0);
    void vertical_scale_span_blit(const point &p, const uint16_t length, surface *texture, const point &st, const point &et);*/
  };

}
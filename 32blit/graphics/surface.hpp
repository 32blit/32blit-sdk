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

  struct SpriteSheet;
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

  enum TextAlign {
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

  struct Surface {

    uint8_t                        *data;                     // pointer to pixel data (for `rgba` format has pre-multiplied alpha)
    Size                            bounds;                   // size of surface in pixels
    
    Rect                            clip;                     // clip rectangle

    RGBA                            _pen;                     // current pen
    uint8_t                         alpha = 255;              // global alpha for drawing operations

    PixelFormat                     format;                   // surface pixel format
    uint8_t                         stride;                   // bytes per pixel
    uint16_t                        row_stride;               // bytes per row

    Surface                        *mask = nullptr;           // mask pointer

    SpriteSheet                    *sprites = nullptr;        // active spritesheet

    std::vector<RGBA>               palette;                  // palette entries (for paletted images)
    uint8_t                         transparent_index = 0;    // index of transparent colour (for paletted surfaces)

    // blend functions
    blit::blend_span_func           bf;
    std::array<BlendBlitFunc, 5>    bbf;
    std::vector<Surface *>          mipmaps;

  private:
    void init();
    void load_from_packed(const packed_image *image);

  public:    
    Surface(uint8_t *data, const PixelFormat &format, const Size &bounds);
    Surface(uint8_t *data, const PixelFormat&format, const packed_image *image);

    Surface *load(const packed_image *image);

    // helpers to retrieve pointer to pixel
    __attribute__((always_inline)) uint8_t* ptr(const Rect &r)   { return data + r.x * stride + r.y * row_stride; }
    __attribute__((always_inline)) uint8_t* ptr(const Point &p)  { return data + p.x * stride + p.y * row_stride; }
    __attribute__((always_inline)) uint8_t* ptr(const int32_t &x, const int32_t &y) { return data + x * stride + y * row_stride; }

    __attribute__((always_inline)) uint32_t offset(const Rect &r) { return r.x + r.y * bounds.w; }
    __attribute__((always_inline)) uint32_t offset(const Point &p) { return p.x + p.y * bounds.w; }
    __attribute__((always_inline)) uint32_t offset(const int32_t &x, const int32_t &y) { return x + y * bounds.w; }    

    void generate_mipmaps(uint8_t depth);

    void pen(RGBA v);

    void clear();
    void pixel(const Point &p);
    void _pixel(const Point &p);
    void _pixel(const uint32_t &o);
    void v_span(Point p, int16_t c);
    void h_span(Point p, int16_t c);
    void rectangle(const Rect &r);
    void circle(const Point &c, int32_t r);

    void line(const Point&p1, const Point&p2);
    void triangle(Point p1, Point p2, Point p3);
    void polygon(std::vector<Point> p);

    void text(std::string message, const uint8_t *font, const Rect &r, bool variable = true, TextAlign align = TextAlign::top_left);
    void text(std::string message, const uint8_t *font, const Point &p, bool variable = true, TextAlign align = TextAlign::top_left);
    Size measure_text(std::string message, const uint8_t *font, bool variable = true);
    std::string wrap_text(std::string message, int32_t width, const uint8_t *font, bool variable = true);

    /*void outline_circle(const point &c, int32_t r);
    
    */ 
    void blit(Surface *src, Rect r, Point p, bool hflip = false);
    void stretch_blit(Surface *src, Rect sr, Rect dr);
    void stretch_blit_vspan(Surface *src, Point uv, uint16_t sc, Point p, int16_t dc);

    void custom_blend(Surface *src, Rect r, Point p, std::function<void(uint8_t *psrc, uint8_t *pdest, int16_t c)> f);
    void custom_modify(Rect r, std::function<void(uint8_t *p, int16_t c)> f);
    void watermark();


    // origin           (optional - default 0, 0)
    // span repeat x/y  (optional - default 1, 1)
    // scale            (optional - default 1)

    //void sprite(spritesheet &ss, uint16_t index, point pos, size span = size(1, 1), point origin = point(0, 0), float scale = 1.0f);
    //void sprite(spritesheet &ss, point sprite, point position, sprite_p &properties);

    
    void blit_sprite(const Rect &src, const Point &p, const uint8_t &t = 0);
    void stretch_blit_sprite(const Rect&src, const Rect &r, const uint8_t &t = 0);

    void sprite(const Rect &sprite, const Point &position, const uint8_t &transform = 0);
    void sprite(const Point &sprite, const Point &position, const uint8_t &transform = 0);
    void sprite(const uint16_t &sprite, const Point &position, const uint8_t &transform = 0);

    void sprite(const Rect &sprite, const Point &position, const Point &origin, const uint8_t &transform = 0);
    void sprite(const Point &sprite, const Point &position, const Point &origin, const uint8_t &transform = 0);
    void sprite(const uint16_t &sprite, const Point &position, const Point &origin, const uint8_t &transform = 0);

    void sprite(const Rect &sprite, const Point &position, const Point &origin, const Vec2 &scale, const uint8_t &transform = 0);
    void sprite(const Point &sprite, const Point &position, const Point &origin, const Vec2 &scale, const uint8_t &transform = 0);
    void sprite(const uint16_t &sprite, const Point &position, const Point &origin, const Vec2 &scale, const uint8_t &transform = 0);
    
    void sprite(const Rect &sprite, const Point &position, const Point &origin, const float &scale, const uint8_t &transform = 0);
    void sprite(const Point &sprite, const Point &position, const Point &origin, const float &scale, const uint8_t &transform = 0);
    void sprite(const uint16_t &sprite, const Point &position, const Point &origin, const float &scale, const uint8_t &transform = 0);

    //extern void texture_triangle(int32_t x1, int32_t y1, int32_t u1, int32_t v1, int32_t x2, int32_t y2, int32_t u2, int32_t v2, int32_t x3, int32_t y3, int32_t u3, int32_t v3);
    
    /*
      blitting methods
    */
    //extern void blit(rgba *src, int16_t sx, int16_t sy, int16_t sw, int16_t w, int16_t h, int16_t dx, int16_t dy, bool hflip);
  /*  void blit(surface *src, rect from, const point &to, uint8_t flip = 0);
    void vertical_scale_span_blit(const point &p, const uint16_t length, surface *texture, const point &st, const point &et);*/
  };

}
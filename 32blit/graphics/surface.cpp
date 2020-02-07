/*! \file surface.cpp
*/
#include <algorithm>
#include <string>

#include "font.hpp"
#include "sprite.hpp"
#include "surface.hpp"

using namespace blit;

namespace blit {


  Surface::Surface(uint8_t *data, const PixelFormat &format, const Size &bounds) : data(data), bounds(bounds), format(format) {
    init();
  }

  Surface::Surface(uint8_t *data, const PixelFormat &format, const packed_image *image) : data(data), format(format) {
    load_from_packed(image);
    init();
  }

  Surface *Surface::load(const packed_image *image) {
    uint8_t *buffer = new uint8_t[pixel_format_stride[image->format] * image->width * image->height];
    return new Surface(buffer, (PixelFormat)image->format, image);
  }

  void Surface::init() {
    clip = Rect(0, 0, bounds.w, bounds.h);

    pixel_stride = pixel_format_stride[static_cast<uint8_t>(format)];
    row_stride = pixel_stride * bounds.w;
    
    switch (format) {
    case PixelFormat::RGBA: {
      pbf = RGBA_RGBA;
      bbf = RGBA_RGBA;
    }break;
    case PixelFormat::RGB: {
      pbf = RGBA_RGB;
      bbf = RGBA_RGB;
    }break;
    case PixelFormat::P: {
      pbf = P_P;
      bbf = P_P;
    }break;
    case PixelFormat::M: {
      pbf = P_P;
      bbf = P_P;
    }break;
    }
  }

  /**
   * Generate mipmaps for surface
   *
   * \param depth `uint8_t`
   */
  void Surface::generate_mipmaps(uint8_t depth) {
    uint16_t w = bounds.w;
    uint16_t h = bounds.h;

    mipmaps.reserve(depth + 1);
    
    Surface *src = this;
    mipmaps.push_back(src);
    
    // offset the data pointer to the end
    uint8_t *mipmap_data = data + (row_stride * bounds.h);

    do {
      w /= 2;
      h /= 2;      
      Surface *dest = new Surface(mipmap_data, PixelFormat::RGBA, Size(w, h));
      mipmaps.push_back(dest);
      
      for (int y = 0; y < h; y++) {      
        for (int x = 0; x < w; x++) {
          // sample the average ARGB values
          Pen *c1, *c2, *c3, *c4;
          if (src->format == PixelFormat::P) {
            c1 = &src->palette[*src->ptr(Point(x * 2, y * 2))];
            c2 = &src->palette[*src->ptr(Point(x * 2 + 1, y * 2))];
            c3 = &src->palette[*src->ptr(Point(x * 2 + 1, y * 2 + 1))];
            c4 = &src->palette[*src->ptr(Point(x * 2, y * 2 + 1))];
          }
          else {
            c1 = (Pen *)(src->ptr(Point(x * 2, y * 2)));
            c2 = (Pen *)(src->ptr(Point(x * 2 + 1, y * 2)));
            c3 = (Pen *)(src->ptr(Point(x * 2 + 1, y * 2 + 1)));
            c4 = (Pen *)(src->ptr(Point(x * 2, y * 2 + 1)));
          }

          uint8_t a = (c1->a + c2->a + c3->a + c4->a) / 4;
          uint8_t r = (c1->r + c2->r + c3->r + c4->r) / 4;
          uint8_t g = (c1->g + c2->g + c3->g + c4->g) / 4;
          uint8_t b = (c1->b + c2->b + c3->b + c4->b) / 4;          
          a = 255;

          dest->pen = Pen(r, g, b, a);
          dest->pixel(Point(x, y));
        }
      }

      src = dest;    
      mipmap_data += (src->row_stride * src->bounds.h);
    } while (--depth);
  }


  /*void surface::blit_sprite(const rect &sprite, const point &p, const uint8_t &t) {
    rect dr = clip.intersection(rect(p.x, p.y, 8, 8));  // clipped destination rect

    if (dr.empty())
      return; // after clipping there is nothing to draw

    uint8_t left = dr.x - p.x;
    uint8_t top = dr.y - p.y;
    uint8_t right = 8 - (8 - dr.w) + left;
    uint8_t bottom = 8 - (8 - dr.h) + top;

    blend_blit_func sbbf = bbf[sprites->format];

    uint32_t dest_offset = offset(dr);    

    for (uint8_t y = top; y < bottom; y++) {
      for (uint8_t x = left; x < right; x++) {
        uint8_t u = x;
        uint8_t v = y;

        if (t) {
          v = (t & sprite_transform::VERTICAL) ? (7 - v) : v;                     // vertical mirror
          u = (t & sprite_transform::HORIZONTAL) ? (7 - u) : u;                   // horizontal mirror
          if (t & sprite_transform::XYSWAP) { uint8_t tmp = u; u = v; v = tmp; }  // axis swap
        }

        uint32_t src_offset = sprites->offset(sprite.x + u, sprite.y + v);

        sbbf(sprites, src_offset, this, dest_offset, 1, 1);
        dest_offset++;
      }

      dest_offset += bounds.w - 8;
    }      
  }*/

  /**
   * Blit a sprite to the surface
   *
   * \param sprite
   * \param p
   * \param t
   */
  void Surface::blit_sprite(const Rect &sprite, const Point &p, const uint8_t &t) {
    Rect dr = clip.intersection(Rect(p.x, p.y, sprite.w, sprite.h));  // clipped destination rect

    if (dr.empty())
      return; // after clipping there is nothing to draw

    uint8_t left = dr.x - p.x;
    uint8_t top = dr.y - p.y;
    uint8_t right = sprite.w - (sprite.w - dr.w) + left - 1;
    uint8_t bottom = sprite.h - (sprite.h - dr.h) + top - 1;    
    
    if (t & SpriteTransform::VERTICAL) {
      top    = sprite.h - 1 - top;
      bottom = sprite.h - 1 - bottom;
    }

    if (t & SpriteTransform::HORIZONTAL) {
      left  = sprite.w - 1 - left;
      right = sprite.w - 1 - right;
    }

    int y_step = top < bottom ? 1 : -1;
    int x_step = left < right ? 1 : -1;

    uint32_t dest_offset = offset(dr);
    uint32_t src_offset;    
    
    uint8_t y_count = dr.h;
    uint8_t y = top;
    do {    
      uint8_t x_count = dr.w;
      uint8_t x = left;
      do {      
        if (t & SpriteTransform::XYSWAP)
          src_offset = sprites->offset(sprite.x + y, sprite.y + x);
        else
          src_offset = sprites->offset(sprite.x + x, sprite.y + y);

        bbf(sprites, src_offset, this, dest_offset, 1, 1);
        dest_offset++;

        x += x_step;
      } while (--x_count);

      dest_offset += bounds.w - dr.w;
      y += y_step;
    } while (--y_count);
  }

  /**
   * Blit a stretched sprite to the surface
   *
   * \param sprite
   * \param p
   * \param t
   */
  void Surface::stretch_blit_sprite(const Rect &sprite, const Rect &r, const uint8_t &t) {
    Rect dr = clip.intersection(r);  // clipped destination rect

    if (dr.empty())
      return; // after clipping there is nothing to draw

    float scale_x = float(sprite.w) / r.w;
    float scale_y = float(sprite.h) / r.h;

    float left = (dr.x - r.x) * scale_x;
    float top = (dr.y - r.y) * scale_y;
    float right = sprite.w - (sprite.w - (dr.w * scale_x)) + left - 1;
    float bottom = sprite.h - (sprite.h - (dr.h * scale_y)) + top - 1;

    if (t & SpriteTransform::VERTICAL) {
      top = sprite.h - 1 - top;
      bottom = sprite.h - 1 - bottom;
    }

    if (t & SpriteTransform::HORIZONTAL) {
      left = sprite.w - 1 - left;
      right = sprite.w - 1 - right;
    }

    float y_step = top < bottom ? scale_y : -scale_y;
    float x_step = left < right ? scale_x : -scale_x;
    
    uint32_t dest_offset = offset(dr);
    uint32_t src_offset;

    uint8_t y_count = dr.h;    
    float y = top;
    do {
      uint8_t x_count = dr.w;
      float x = left;
      do {
        if (t & SpriteTransform::XYSWAP)
          src_offset = sprites->offset(sprite.x + y, sprite.y + x);
        else
          src_offset = sprites->offset(sprite.x + x, sprite.y + y);

        bbf(sprites, src_offset, this, dest_offset, 1, 1);
        dest_offset++;

        x += x_step;
      } while (--x_count);

      dest_offset += bounds.w - dr.w;
      y += y_step;
    } while (--y_count);
  }

  /**
   * Blit another surface to the surface
   *
   * \param src
   * \param r
   * \param p
   * \param hflip `true` to flip the source surface horizontally
   */
  void Surface::blit(Surface *src, Rect r, Point p, bool hflip) {    
    Rect dr = clip.intersection(Rect(p.x, p.y, r.w, r.h));  // clipped destination rect

    if (dr.empty()) 
      return; // after clipping there is nothing to draw

    // offset source rect to accomodate for clipped destination rect    
    uint8_t l = dr.x - p.x; // top left corner
    uint8_t t = dr.y - p.y; 
    r.x += l; r.w -= l; r.y += t; r.h -= t;    
    r.w = dr.w; // clamp width/height
    r.h = dr.h;

    uint32_t src_offset = src->offset(r.x, r.y);

    uint8_t src_offset_flip = 0;
    int8_t src_direction = 1;
    if (hflip) {
      src_offset_flip = r.w - 1;
      src_direction = -1;
    }

    
    
    int32_t dest_offset = offset(dr);
    for (int32_t y = p.y; y < p.y + r.h; y++) {
      bbf(src, src_offset + src_offset_flip, this, dest_offset, r.w, src_direction);

      src_offset += src->bounds.w;
      dest_offset += bounds.w;      
    }
  }

  /**
   * Blit from another surface
   *
   * \param src
   * \param sr `rect` soruce
   * \param dr `rect` destination
   */
  void Surface::stretch_blit(Surface *src, Rect sr, Rect dr) {
    Rect cdr = clip.intersection(dr);  // clipped destination rect

    if (cdr.empty())
      return; // after clipping there is nothing to draw

    float sx = (sr.w) / float(dr.w);
    float sy = (sr.h) / float(dr.h);

    // offset source rect to accomodate for clipped destination rect    
    uint8_t l = cdr.x - dr.x; // top left corner
    uint8_t t = cdr.y - dr.y;

    sr.x += (sx * l);
    sr.y += (sy * t);

    sr.w = cdr.w * sx;
    sr.h = cdr.h * sy;

    /*BlendBlitFunc sbbf = bbf[static_cast<uint8_t>(src->format)];

    float src_y = sr.y;
    for (int32_t y = cdr.y; y < cdr.y + cdr.h; y++) {
      float src_x = sr.x;
      for (int32_t x = cdr.x; x < cdr.x + cdr.w; x++) {
        sbbf(src, src->offset(src_x, src_y), this, offset(x, y), 1, 1);

        src_x += sx;
      }      
      src_y += sy;
    }*/
  }

  /**
   * Blit a vertical span
   *
   * \param src
   * \param uv
   * \param sc
   * \param p
   * \param dc
   */
  void Surface::stretch_blit_vspan(Surface *src, Point uv, uint16_t sc, Point p, int16_t dc) {
    float v = uv.y;
    float vs = float(sc) / float(dc);

    if (p.y < 0) {
      dc += p.y;
      v += (vs * float(-p.y));
      p.y = 0;
    }

    if (dc <= 0) {
      return;
    }

    /*BlendBlitFunc sbbf = bbf[static_cast<uint8_t>(src->format)];

    int16_t max_y = std::min(p.y + dc, bounds.h);
    for (; p.y < max_y; p.y++) {
      sbbf(src, src->offset(Point(uv.x, v)), this, offset(p), 1, 1);

      v += vs;
    }*/
  }

  /**
   * TODO: Document this function
   *
   * \param src
   * \param r
   * \param p
   * \param f
   */
  void Surface::custom_blend(Surface *src, Rect r, Point p, std::function<void(uint8_t *psrc, uint8_t *pdest, int16_t c)> f) {
    Rect dr = clip.intersection(Rect(p.x, p.y, r.w, r.h));  // clipped destination rect

    if (dr.empty())
      return; // after clipping there is nothing to draw 

    // offset source rect to accomodate for clipped destination rect    
    uint8_t l = dr.x - p.x; // top left corner
    uint8_t t = dr.y - p.y;
    r.x += l; r.w -= l; r.y += t; r.h -= t;
    r.w = dr.w; // clamp width/height
    r.h = dr.h;

    uint8_t *psrc = src->ptr(r.x, r.y);
    uint8_t *pdest = ptr(dr.x, dr.y);
    
    for (int32_t y = 0; y < dr.h; y++) {
      f(psrc, pdest, dr.w);      

      psrc += src->bounds.w;
      pdest += bounds.w;
    }
  }

  /**
   * TODO: Document this function
   *
   * \param r
   * \param f
   */
  void Surface::custom_modify(Rect r, std::function<void(uint8_t *p, int16_t c)> f) {
    Rect dr = clip.intersection(r);  // clipped destination rect

    if (dr.empty())
      return; // after clipping there is nothing to draw

    uint8_t *p = ptr(dr.x, dr.y);

    for (int32_t y = 0; y < dr.h; y++) {
      f(p, dr.w);

      p += bounds.w;
    }
  }

  /**
   * TODO: Document this function
   *
   * \param image
   */
  void Surface::load_from_packed(const packed_image *image) {        
    uint8_t *palette_entries = ((uint8_t *)image) + sizeof(packed_image);
    uint8_t *bytes = ((uint8_t *)image) + sizeof(packed_image) + (image->palette_entry_count * 4);

    bounds = Size(image->width, image->height);

    uint8_t bit_depth = uint8_t(ceil(log(image->palette_entry_count) / log(2)));

    uint8_t col = 0;
    uint8_t bit = 0;

    if (format == PixelFormat::P) {
      // load paletted
      uint8_t *pdest = (uint8_t *)data;
      
      palette = new Pen[256];
      for (uint8_t pidx = 0; pidx < image->palette_entry_count; pidx++) {
        palette[pidx] = Pen(
          palette_entries[pidx * 4 + 0],
          palette_entries[pidx * 4 + 1],
          palette_entries[pidx * 4 + 2],
          palette_entries[pidx * 4 + 3]);
      }

      for (uint8_t b = *bytes; bytes < ((uint8_t *)image) + image->byte_count; b = *++bytes) {
        for (auto j = 0; j < 8; j++) {
          col <<= 1;
          col |= ((0b10000000 >> j) & b) ? 1 : 0;

          bit++;
          if (bit == bit_depth) {
            *pdest++ = col;            
            bit = 0; col = 0;
          }
        }
      }
    }else{
      Pen *pdest = (Pen *)data;

      for (uint8_t b = *bytes; bytes < ((uint8_t *)image) + image->byte_count; b = *++bytes) {
        for (auto j = 0; j < 8; j++) {
          col <<= 1;
          col |= ((0b10000000 >> j) & b) ? 1 : 0;

          bit++;
          if (bit == bit_depth) {
            *pdest++ = Pen(
              palette_entries[col * 4 + 0],
              palette_entries[col * 4 + 1],
              palette_entries[col * 4 + 2],
              palette_entries[col * 4 + 3]);

            bit = 0; col = 0;
          }
        }
      }
    }
  }

  /**
   * TODO: Document this function
   */
  void Surface::watermark() {
    static uint8_t logo[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0,
        0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 0,
        0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 1, 0, 2, 0, 1, 0, 0,
        0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0,
        0, 2, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0,
        0, 2, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0,
        0, 2, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    static Pen pens[] = { Pen(39, 39, 56), Pen(255, 255, 255), Pen(0, 255, 0) };

    uint8_t scale = bounds.w / 160;
    for (uint8_t y = 0; y < 13; y++) {
      for (uint8_t x = 0; x < 13; x++) {
        Pen &p = pens[logo[x + y * 13]];
        int o = offset(bounds.w - (15 * scale) + (x * scale), bounds.h - (15 * scale) + (y * scale));
        pbf(&p, this, o, scale);
        if (scale == 2) {
          pbf(&p, this, o + bounds.w, scale);
        }
      }
    }
  }
}


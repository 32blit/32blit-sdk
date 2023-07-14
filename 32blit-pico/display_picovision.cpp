#include "display.hpp"

#include "config.h"

#include "dv_display.hpp"

static pimoroni::DVDisplay display;

static volatile bool do_render = false;

static uint16_t blend_buf[256];

static void vsync_callback(uint gpio, uint32_t events){
  if(!do_render) {
    display.flip();
    do_render = true;
  }
}

// these three are copied from blend.cpp
inline uint32_t alpha(uint32_t a1, uint32_t a2) {
  return ((a1 + 1) * (a2 + 1)) >> 8;
}

inline uint32_t alpha(uint32_t a1, uint32_t a2, uint32_t a3) {
  return ((a1 + 1) * (a2 + 1) * (a3 + 1)) >> 16;
}

inline uint8_t blend(uint8_t s, uint8_t d, uint8_t a) {
  return d + ((a * (s - d) + 127) >> 8);
}

inline uint16_t pack_rgb555(uint8_t r, uint8_t g, uint8_t b) {
  return (b >> 3) | ((g >> 3) << 5) | ((r >> 3) << 10);
}

inline void unpack_rgb555(uint16_t rgb555, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (rgb555 >> 10) & 0x1F; r = r << 3;
  g = (rgb555 >>  5) & 0x1F; g = g << 3;
  b =  rgb555        & 0x1F; b = b << 3;
}

inline void blend_rgba_rgb555(const blit::Pen* s, uint32_t off, int dest_w, uint8_t a, uint32_t c) {
  do {
    auto step = std::min(c, uint32_t(std::size(blend_buf)));

    int x = off % dest_w;
    int y = off / dest_w;

    off += step;
    c -= step;

    display.read_pixel_span({x, y}, step, blend_buf);

    auto *ptr = blend_buf;
    for(unsigned i = 0; i < step; i++) {
      uint8_t r, g, b;
      unpack_rgb555(*ptr, r, g, b);

      *ptr++ = pack_rgb555(blend(s->r, r, a), blend(s->g, g, a), blend(s->b, b, a));
    }

    display.write_pixel_span({x, y}, step, blend_buf);
  } while(c);
}

static void pen_rgba_rgb555_picovision(const blit::Pen* pen, const blit::Surface* dest, uint32_t off, uint32_t c) {
  if(!pen->a) return;

  uint8_t* m = dest->mask ? dest->mask->data + off : nullptr;

  uint16_t a = alpha(pen->a, dest->alpha);

  auto pen555 = pack_rgb555(pen->r, pen->g, pen->b);

  if (!m) {
    int x = off % dest->bounds.w;
    int y = off / dest->bounds.w;

    // no mask
    if (a >= 255) {
      // no alpha, just copy
      display.write_pixel_span({x, y}, c, pen555);
    }
    else {
      // alpha, blend
      blend_rgba_rgb555(pen, off, dest->bounds.w, a, c);
    }
  } else {
    // mask enabled, slow blend
    do {
      uint16_t ma = alpha(a, *m++);
      blend_rgba_rgb555(pen, off, dest->bounds.w, ma, 1);
      off++;
    } while (--c);
  }
}

static void blit_rgba_rgb555_picovision(const blit::Surface* src, uint32_t soff, const blit::Surface* dest, uint32_t doff, uint32_t cnt, int32_t src_step) {
  uint8_t* s = src->palette ? src->data + soff : src->data + (soff * src->pixel_stride);
  uint8_t* m = dest->mask ? dest->mask->data + doff : nullptr;

  do {
    auto step = std::min(cnt, uint32_t(std::size(blend_buf)));

    int x = doff % dest->bounds.w;
    int y = doff / dest->bounds.w;

    doff += step;
    cnt -= step;

    // TODO: only if needed
    if(src->format != blit::PixelFormat::RGB)
      display.read_pixel_span({x, y}, step, blend_buf);

    auto *ptr = blend_buf;
    for(unsigned i = 0; i < step; i++) {
      auto pen = src->palette ? &src->palette[*s] : (blit::Pen *)s;

      uint16_t a = src->format == blit::PixelFormat::RGB ? 255 : pen->a;
      a = m ? alpha(a, *m++, dest->alpha) : alpha(a, dest->alpha);

      if(a >= 255) {
        *ptr++ = pack_rgb555(pen->r, pen->g, pen->b);
      } else if (a > 1) {
        uint8_t r, g, b;
        unpack_rgb555(*ptr, r, g, b);
        *ptr++ = pack_rgb555(blend(pen->r, r, a), blend(pen->g, g, a), blend(pen->b, b, a));
      }else{
        ptr++;
      }

      s += (src->pixel_stride) * src_step;
    }

    display.write_pixel_span({x, y}, step, blend_buf);

  } while(cnt);
}

void init_display() {
  display.init(DISPLAY_WIDTH, DISPLAY_HEIGHT, pimoroni::DVDisplay::MODE_RGB555);

  gpio_set_irq_enabled_with_callback(16/*VSYNC*/, GPIO_IRQ_EDGE_RISE, true, vsync_callback);
}

void update_display(uint32_t time) {
  if(!do_render)
    return;
  
  blit::render(time);

  do_render = false;
}

void init_display_core1() {
}

void update_display_core1() {
}

bool display_render_needed() {
  return do_render;
}

bool display_mode_supported(blit::ScreenMode new_mode, const blit::SurfaceTemplate &new_surf_template) {
  if(new_surf_template.format != blit::PixelFormat::RGB565) // this is a lie
    return false;

  // TODO
  blit::Size base_bounds(640, 480);

  if(new_surf_template.bounds == base_bounds || new_surf_template.bounds == base_bounds / 2 || new_surf_template.bounds == base_bounds / 4)
    return true;

  return false;
}

void display_mode_changed(blit::ScreenMode new_mode, blit::SurfaceTemplate &new_surf_template) {
  new_surf_template.pen_blend = pen_rgba_rgb555_picovision;
  new_surf_template.blit_blend = blit_rgba_rgb555_picovision;
}

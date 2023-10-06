#include "display.hpp"

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "aps6404.hpp"
#include "swd_load.hpp"
#include "pico-stick.h"

#include "config.h"

// pins
static constexpr uint CS     = 17;
static constexpr uint D0     = 19;
static constexpr uint VSYNC  = 16;
static constexpr uint RAM_SEL = 8;

static constexpr uint I2C_SDA = 6;
static constexpr uint I2C_SCL = 7;

// i2c
static constexpr uint I2C_ADDR = 0x0D;
static constexpr uint I2C_REG_SET_RES = 0xFC;
static constexpr uint I2C_REG_START = 0xFD;

static constexpr uint32_t base_address = 0x10000;

static const blit::Size resolutions[]{
  {640, 480},
};

static pimoroni::APS6404 ram(CS, D0, pio1);
static uint8_t ram_bank = 0;

static bool display_enabled = false;
static uint8_t need_mode_change = 2;
static int cur_resolution = 0;

static volatile bool do_render = true;

static uint16_t blend_buf[256];

static uint32_t batch_start_addr = 0, batch_next_off = ~0u;
static uint16_t *batch_ptr = nullptr;
static const uint16_t *batch_end = blend_buf + std::size(blend_buf);

static void vsync_callback(uint gpio, uint32_t events){
  if(!do_render) {
    ram_bank ^= 1;
    gpio_put(RAM_SEL, ram_bank);

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

static void flush_batch() {
  if(batch_ptr)
    ram.write(batch_start_addr, (uint32_t *)blend_buf, (batch_ptr - blend_buf) * 2);

  batch_ptr = nullptr;
  batch_next_off = ~0u;
}

inline void blend_rgba_rgb555(const blit::Pen* s, uint32_t off, uint8_t a, uint32_t c) {
  flush_batch();
  do {
    auto step = std::min(c, uint32_t(std::size(blend_buf)));

    ram.read_blocking(base_address + off * 2, (uint32_t*)blend_buf, (step + 1) >> 1);

    auto *ptr = blend_buf;
    for(unsigned i = 0; i < step; i++) {
      uint8_t r, g, b;
      unpack_rgb555(*ptr, r, g, b);

      *ptr++ = pack_rgb555(blend(s->r, r, a), blend(s->g, g, a), blend(s->b, b, a));
    }

    ram.write(base_address + off * 2, (uint32_t *)blend_buf, step * 2);

    off += step;
    c -= step;
  } while(c);
}

[[gnu::always_inline]]
inline void copy_rgba_rgb555(const blit::Pen* s, uint32_t off, uint32_t c) {
  auto pen555 = pack_rgb555(s->r, s->g, s->b);

  constexpr size_t cache_size = std::size(blend_buf);

  if(c >= cache_size) {
    // big fill, skip the batch buf
    flush_batch();

    uint32_t val = pen555 | pen555 << 16;
    do {
      auto step = std::min(c, UINT32_C(512));
      ram.write_repeat(base_address + off * 2, val, step * 2);
      off += step;
      c -= step;
    } while(c);
  } else {
    // batching
    if(batch_next_off != off || batch_ptr + c > batch_end) {
      flush_batch();

      batch_start_addr = base_address + off * 2;
      batch_next_off = off;
      batch_ptr = blend_buf;
    }

    // write to cache buf
    batch_next_off += c;

    do {
      *batch_ptr++ = pen555;
    } while(--c);
  }
}

template<int h_repeat = 1>
static void pen_rgba_rgb555_picovision(const blit::Pen* pen, const blit::Surface* dest, uint32_t off, uint32_t c) {
  if(!pen->a) return;

  uint8_t* m = dest->mask ? dest->mask->data + off : nullptr;

  uint16_t a = alpha(pen->a, dest->alpha);

  off *= h_repeat;
  c *= h_repeat;

  if (!m) {
    // no mask
    if (a >= 255) {
      // no alpha, just copy
      copy_rgba_rgb555(pen, off, c);
    }
    else {
      // alpha, blend
      blend_rgba_rgb555(pen, off, a, c);
    }
  } else {
    // mask enabled, slow blend
    do {
      uint16_t ma = alpha(a, *m++);
      blend_rgba_rgb555(pen, off, ma, 1);
      off++;
    } while (--c);
  }
}

template<int h_repeat = 1>
static void blit_rgba_rgb555_picovision(const blit::Surface* src, uint32_t soff, const blit::Surface* dest, uint32_t doff, uint32_t cnt, int32_t src_step) {
  uint8_t* s = src->palette ? src->data + soff : src->data + (soff * src->pixel_stride);
  uint8_t* m = dest->mask ? dest->mask->data + doff : nullptr;

  doff *= h_repeat;
  cnt *= h_repeat;

  flush_batch();

  do {
    auto step = std::min(cnt, uint32_t(std::size(blend_buf)));

    // TODO: only if needed
    if(src->format != blit::PixelFormat::RGB)
      ram.read_blocking(base_address + doff * 2, (uint32_t*)blend_buf, (step + 1) >> 1);

    auto *ptr = blend_buf;
    for(unsigned i = 0; i < step; i += h_repeat) {
      auto pen = src->palette ? &src->palette[*s] : (blit::Pen *)s;

      uint16_t a = src->format == blit::PixelFormat::RGB ? 255 : pen->a;
      a = m ? alpha(a, *m++, dest->alpha) : alpha(a, dest->alpha);

      for(int j = 0; j < h_repeat; j++) {
        if(a >= 255) {
          *ptr++ = pack_rgb555(pen->r, pen->g, pen->b);
        } else if (a > 1) {
          uint8_t r, g, b;
          unpack_rgb555(*ptr, r, g, b);
          *ptr++ = pack_rgb555(blend(pen->r, r, a), blend(pen->g, g, a), blend(pen->b, b, a));
        }else{
          ptr++;
        }
      }

      s += (src->pixel_stride) * src_step;
    }

    ram.write(base_address + doff * 2, (uint32_t *)blend_buf, step * 2);

    doff += step;
    cnt -= step;

  } while(cnt);
}

template<int h_repeat = 1>
static blit::Pen get_pen_rgb555_picovision(const blit::Surface *surf, uint32_t offset) {
  uint32_t tmp;
  ram.read_blocking(base_address + offset * h_repeat * 2, &tmp, 1);

  uint8_t r, g, b;
  unpack_rgb555(tmp, r, g, b);
  return {r, g, b};
}

static void write_frame_setup(uint16_t width, uint16_t height, blit::PixelFormat format, uint8_t h_repeat, uint8_t v_repeat) {
  constexpr int buf_size = 32;
  uint32_t buf[buf_size];

  int dv_format = 1; // 555

  uint32_t full_width = width * h_repeat;
  buf[0] = 0x4F434950; // "PICO"

  // setup
  buf[1] = 0x01000101 + ((uint32_t)v_repeat << 16);
  buf[2] = full_width << 16;
  buf[3] = (uint32_t)height << 16;

  // frame table header
  buf[4] = 0x00000001; // 1 frame, start at frame 0
  buf[5] = 0x00010000 + height + ((uint32_t)ram_bank << 24); // frame rate divider 1
  buf[6] = 0x00000001; // 1 palette, don't advance, 0 sprites

  ram.write(0, buf, 7 * 4);
  ram.wait_for_finish_blocking();

  // write frame table
  uint frame_table_addr = 4 * 7;
  
  for(int y = 0; y < height; y += buf_size) {
    int step = std::min(buf_size, height - y);
    for(int i = 0; i < step; i++) {
      uint32_t line_addr = base_address + (y + i) * width * blit::pixel_format_stride[int(format)];
      buf[i] = dv_format << 27 | h_repeat << 24 | line_addr;
    }

    ram.write(frame_table_addr, buf, step * 4);
    ram.wait_for_finish_blocking();
    frame_table_addr += 4 * step;
  }
}

void init_display() {
  gpio_init(RAM_SEL);
  gpio_put(RAM_SEL, 0);
  gpio_set_dir(RAM_SEL, GPIO_OUT);

  gpio_init(VSYNC);
  gpio_set_dir(VSYNC, GPIO_IN);
  gpio_set_irq_enabled_with_callback(VSYNC, GPIO_IRQ_EDGE_RISE, true, vsync_callback);

  sleep_ms(200);
  swd_load_program(section_addresses, section_data, section_data_len, std::size(section_data_len), 0x20000001, 0x15004000, true);

  // init RAM
  ram.init();
  sleep_us(100);

  gpio_put(RAM_SEL, 1);
  ram.init();
  sleep_us(100);

  ram_bank = 0;
  gpio_put(RAM_SEL, 0);
  sleep_ms(100);

  // i2c init
  i2c_init(i2c1, 400000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SCL);

  uint8_t resolution = 0; // 640x480
  uint8_t buf[2] = {I2C_REG_SET_RES, resolution};
  i2c_write_blocking(i2c1, I2C_ADDR, buf, 2, false);
}

static int find_resolution(const blit::Size &bounds) {
  int i = 0;

  for(auto &res : resolutions) {
    if(bounds == res || bounds == res / 2 || bounds == res / 4)
      return i;

    i++;
  }
  
  return -1;
}

void update_display(uint32_t time) {
  if(!do_render)
    return;
  
  blit::render(time);

  flush_batch();

  ram.wait_for_finish_blocking();

  // handle mode change
  if(need_mode_change) {
    auto &cur_surf_info = blit::screen;

    auto new_res = find_resolution(cur_surf_info.bounds);

    auto &base_bounds = resolutions[new_res];

    uint8_t h_repeat = base_bounds.w / cur_surf_info.bounds.w, v_repeat = base_bounds.h / cur_surf_info.bounds.h;

    uint16_t final_w = cur_surf_info.bounds.w;

    if(h_repeat > 2) {
      h_repeat = 2;
      final_w = base_bounds.w / 2;
    }

    write_frame_setup(final_w,  cur_surf_info.bounds.h,  cur_surf_info.format, h_repeat, v_repeat);
    need_mode_change--;
  }

  // enable display after first render
  if(!display_enabled) {
    // swap banks now
    ram_bank ^= 1;
    gpio_put(RAM_SEL, ram_bank);

    uint8_t buf[2] = {I2C_REG_START, 1};
    i2c_write_blocking(i2c1, I2C_ADDR, buf, 2, false);
    display_enabled = true;
  } else
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
  if(new_surf_template.format != blit::PixelFormat::BGR555)
    return false;

  if(find_resolution(new_surf_template.bounds) != -1)
    return true;

  return false;
}

void display_mode_changed(blit::ScreenMode new_mode, blit::SurfaceTemplate &new_surf_template) {
  if(new_surf_template.bounds.w <= 160) {
    new_surf_template.pen_blend = pen_rgba_rgb555_picovision<2>;
    new_surf_template.blit_blend = blit_rgba_rgb555_picovision<2>;
    new_surf_template.pen_get = get_pen_rgb555_picovision<2>;
  } else {
    new_surf_template.pen_blend = pen_rgba_rgb555_picovision;
    new_surf_template.blit_blend = blit_rgba_rgb555_picovision;
    new_surf_template.pen_get = get_pen_rgb555_picovision;
  }

  need_mode_change = 2; // make sure to update both banks
}

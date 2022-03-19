#include "display.hpp"

#include "config.h"

#ifdef DISPLAY_ST7789
#include "st7789.hpp"
#elif defined(DISPLAY_SCANVIDEO)
#include "hardware/clocks.h"
#include "pico/time.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#endif

using namespace blit;

static SurfaceInfo cur_surf_info;

#ifdef DISPLAY_ST7789
// height rounded up to handle the 135px display
static const int lores_page_size = (DISPLAY_WIDTH / 2) * ((DISPLAY_HEIGHT + 1) / 2) * 2;

#if ALLOW_HIRES
static uint16_t screen_fb[DISPLAY_WIDTH * DISPLAY_HEIGHT];
#else
static uint16_t screen_fb[lores_page_size]; // double-buffered
#endif
static bool have_vsync = false;
static bool backlight_enabled = false;

static const blit::SurfaceTemplate lores_screen{(uint8_t *)screen_fb, Size(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2), blit::PixelFormat::RGB565, nullptr};
static const blit::SurfaceTemplate hires_screen{(uint8_t *)screen_fb, Size(DISPLAY_WIDTH, DISPLAY_HEIGHT), blit::PixelFormat::RGB565, nullptr};

#elif defined(DISPLAY_SCANVIDEO)
static uint16_t screen_fb[160 * 120 * 2];
static const blit::SurfaceTemplate lores_screen{(uint8_t *)screen_fb, Size(160, 120), blit::PixelFormat::RGB565, nullptr};
#endif

static ScreenMode cur_screen_mode = ScreenMode::lores;
// double buffering for lores
static volatile int buf_index = 0;

static volatile bool do_render = true;
static uint32_t last_render = 0;

// user render function
void render(uint32_t);

#ifdef DISPLAY_ST7789
static void vsync_callback(uint gpio, uint32_t events) {
  if(!do_render && !st7789::dma_is_busy()) {
    st7789::update();
    do_render = true;
  }
}
#endif

#ifdef DISPLAY_SCANVIDEO

static void fill_scanline_buffer(struct scanvideo_scanline_buffer *buffer) {
  static uint32_t postamble[] = {
    0x0000u | (COMPOSABLE_EOL_ALIGN << 16)
  };

  int w = screen.bounds.w;

  buffer->data[0] = 4;
  buffer->data[1] = host_safe_hw_ptr(buffer->data + 8);
  buffer->data[2] = (w - 4) / 2; // first four pixels are handled separately
  uint16_t *pixels = screen_fb + buf_index * (160 * 120) + scanvideo_scanline_number(buffer->scanline_id) * w;
  buffer->data[3] = host_safe_hw_ptr(pixels + 4);
  buffer->data[4] = count_of(postamble);
  buffer->data[5] = host_safe_hw_ptr(postamble);
  buffer->data[6] = 0;
  buffer->data[7] = 0;
  buffer->data_used = 8;

  // 3 pixel run followed by main run, consuming the first 4 pixels
  buffer->data[8] = (pixels[0] << 16u) | COMPOSABLE_RAW_RUN;
  buffer->data[9] = (pixels[1] << 16u) | 0;
  buffer->data[10] = (COMPOSABLE_RAW_RUN << 16u) | pixels[2];
  buffer->data[11] = (((w - 3) + 1 - 3) << 16u) | pixels[3]; // note we add one for the black pixel at the end
}

static int64_t timer_callback(alarm_id_t alarm_id, void *user_data) {
  static int last_frame = 0;
  struct scanvideo_scanline_buffer *buffer = scanvideo_begin_scanline_generation(false);
  while (buffer) {
    fill_scanline_buffer(buffer);
    scanvideo_end_scanline_generation(buffer);

    auto next_frame = scanvideo_frame_number(scanvideo_get_next_scanline_id());
    if(next_frame != last_frame) {
    //if(scanvideo_in_vblank() && !do_render) {
      do_render = true;
      last_frame = next_frame;
      break;
    }

    buffer = scanvideo_begin_scanline_generation(false);
  }

  return 100;
}
#endif

void init_display() {
#ifdef DISPLAY_ST7789
  st7789::frame_buffer = screen_fb;
  st7789::init();
  st7789::clear();

  have_vsync = st7789::vsync_callback(vsync_callback);
#endif

#ifdef DISPLAY_SCANVIDEO
  //scanvideo_setup(&vga_mode_320x240_60); // not quite
  scanvideo_setup(&vga_mode_160x120_60);
  scanvideo_timing_enable(true);
  add_alarm_in_us(100, timer_callback, nullptr, true);
#endif
}

void update_display(uint32_t time) {

#ifdef DISPLAY_ST7789
  if((do_render || (!have_vsync && time - last_render >= 20)) && (cur_screen_mode == ScreenMode::lores || !st7789::dma_is_busy())) {
    if(cur_screen_mode == ScreenMode::lores) {
      buf_index ^= 1;

      screen.data = (uint8_t *)screen_fb + (buf_index) * lores_page_size;
      st7789::frame_buffer = (uint16_t *)screen.data;
    }

    ::render(time);

    if(!have_vsync) {
      while(st7789::dma_is_busy()) {} // may need to wait for lores.
      st7789::update();
    }

    if(last_render && !backlight_enabled) {
      // the first render should have made it to the screen at this point
      st7789::set_backlight(255);
      backlight_enabled = true;
    }

    last_render = time;
    do_render = false;
  }

#elif defined(DISPLAY_SCANVIDEO)
  if(do_render) {
    screen.data = (uint8_t *)screen_fb + (buf_index ^ 1) * (160 * 120 * 2); // only works because there's no "firmware" here
    ::render(time);
    buf_index ^= 1;
    do_render = false;
  }
#endif
}

bool display_render_needed() {
  return do_render;
}

// blit api

SurfaceInfo &set_screen_mode(ScreenMode mode) {
  switch(mode) {
    case ScreenMode::lores:
      cur_surf_info = lores_screen;
      // window
#ifdef DISPLAY_ST7789
      if(have_vsync)
        do_render = true; // prevent starting an update during switch

      st7789::set_pixel_double(true);
#endif
      break;

    case ScreenMode::hires:
#if defined(DISPLAY_ST7789) && ALLOW_HIRES
      if(have_vsync)
        do_render = true;

      cur_surf_info = hires_screen;
      st7789::frame_buffer = screen_fb;
      st7789::set_pixel_double(false);
#else
      return cur_surf_info;
#endif
      break;

    //case ScreenMode::hires_palette:
    //  screen = hires_palette_screen;
    //  break;
  }

  cur_screen_mode = mode;

  return cur_surf_info;
}

bool set_screen_mode_format(ScreenMode new_mode, SurfaceTemplate &new_surf_template) {
  new_surf_template.data = (uint8_t *)screen_fb;

  switch(new_mode) {
    case ScreenMode::lores:
      new_surf_template.bounds = lores_screen.bounds;
      break;
    case ScreenMode::hires:
    case ScreenMode::hires_palette:
#if defined(DISPLAY_ST7789) && ALLOW_HIRES
      new_surf_template.bounds = hires_screen.bounds;
      break;
#else
      return false; // no hires for scanvideo
#endif
  }

#ifdef DISPLAY_ST7789
      if(have_vsync)
        do_render = true; // prevent starting an update during switch

      st7789::set_pixel_double(new_mode == ScreenMode::lores);

      if(new_mode == ScreenMode::hires)
        st7789::frame_buffer = screen_fb;
#endif

  // don't support any other formats for various reasons (RAM, no format conversion, pixel double PIO)
  if(new_surf_template.format != PixelFormat::RGB565)
    return false;

  cur_screen_mode = new_mode;

  return true;
}

void set_screen_palette(const Pen *colours, int num_cols) {

}

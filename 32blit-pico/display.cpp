#include "display.hpp"

#include "config.h"

using namespace blit;

static SurfaceInfo cur_surf_info;

bool fb_double_buffer = true;

#if defined(BLIT_BOARD_PIMORONI_PICOVISION)
static const uint16_t *screen_fb = nullptr;
#elif ALLOW_HIRES
uint16_t screen_fb[DISPLAY_WIDTH * DISPLAY_HEIGHT];
#else
uint16_t screen_fb[lores_page_size]; // double-buffered
#endif

static const Size lores_screen_size(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);
static const Size hires_screen_size(DISPLAY_WIDTH, DISPLAY_HEIGHT);

ScreenMode cur_screen_mode = ScreenMode::lores;
// double buffering for lores
static volatile int buf_index = 0;

static volatile bool do_render = true;

// blit api

SurfaceInfo &set_screen_mode(ScreenMode mode) {
  SurfaceTemplate temp{nullptr, {0, 0}, mode == ScreenMode::hires_palette ? PixelFormat::P : DEFAULT_SCREEN_FORMAT};

  // may fail for hires/palette
  if(set_screen_mode_format(mode, temp)) {
    cur_surf_info.data = temp.data;
    cur_surf_info.bounds = temp.bounds;
    cur_surf_info.format = temp.format;
    cur_surf_info.palette = temp.palette;
  }

  return cur_surf_info;
}

bool set_screen_mode_format(ScreenMode new_mode, SurfaceTemplate &new_surf_template) {
  new_surf_template.data = (uint8_t *)screen_fb;

  if(new_surf_template.format == (PixelFormat)-1)
    new_surf_template.format = DEFAULT_SCREEN_FORMAT;

  switch(new_mode) {
    case ScreenMode::lores:
      if(new_surf_template.bounds.empty())
        new_surf_template.bounds = lores_screen_size;
      else
        new_surf_template.bounds /= 2;

      break;
    case ScreenMode::hires:
    case ScreenMode::hires_palette:
#if ALLOW_HIRES
      if(new_surf_template.bounds.empty())
        new_surf_template.bounds = hires_screen_size;

      break;
#else
      return false; // no hires for scanvideo
#endif
  }

  // check the framebuffer is large enough for mode
  auto fb_size = uint32_t(new_surf_template.bounds.area()) * pixel_format_stride[int(new_surf_template.format)];

  if(!display_mode_supported(new_mode, new_surf_template))
    return false;

  fb_double_buffer = fb_size * 2 <= sizeof(screen_fb);

  display_mode_changed(new_mode, new_surf_template);

  cur_screen_mode = new_mode;

  return true;
}

void set_screen_palette(const Pen *colours, int num_cols) {

}

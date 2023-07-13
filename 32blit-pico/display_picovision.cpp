#include "display.hpp"

#include "config.h"

#include "dv_display.hpp"

static pimoroni::DVDisplay display;

static volatile bool do_render = false;

static void vsync_callback(uint gpio, uint32_t events){
  if(!do_render) {
    display.flip();
    do_render = true;
  }
}

void init_display() {
  display.init(DISPLAY_WIDTH, DISPLAY_HEIGHT, pimoroni::DVDisplay::MODE_RGB555);

  gpio_set_irq_enabled_with_callback(16/*VSYNC*/, GPIO_IRQ_EDGE_RISE, true, vsync_callback);
}

void update_display(uint32_t time) {
  if(!do_render)
    return;
  
  blit::render(time);

  uint16_t line555[DISPLAY_WIDTH]{};

  bool is_lores = cur_screen_mode == blit::ScreenMode::lores;

  auto &cur_surf_info = blit::screen;

  for(int y = 0; y < cur_surf_info.bounds.h; y++) {
    auto line = (uint16_t *)blit::screen.data + y * cur_surf_info.bounds.w;

    auto out = line555;

    for(int x = 0; x < cur_surf_info.bounds.w; x++) {
      *out++ = (line[x] << 10 & 0x7C00) | (line[x] >> 1 & 0x03E0) | (line[x] >> 11 & 0x001F);
      if(is_lores) {
        *out = *(out - 1);
        out++;
      }
    }

    if(is_lores) {
      display.write_pixel_span({0, y * 2}, DISPLAY_WIDTH, line555);
      display.write_pixel_span({0, y * 2 + 1}, DISPLAY_WIDTH, line555);
    } else
      display.write_pixel_span({0, y}, DISPLAY_WIDTH, line555);
  }

  sleep_us(15); // definitely not a hack for sync

  //display.flip(); // waits for vsync
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
}

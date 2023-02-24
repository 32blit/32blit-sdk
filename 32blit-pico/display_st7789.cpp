#include "display.hpp"

#include "config.h"

#include "st7789.hpp"

using namespace blit;

// double buffering for lores
static volatile int buf_index = 0;

static volatile bool do_render = true;

static bool have_vsync = false;
static bool backlight_enabled = false;
static uint32_t last_render = 0;

static void vsync_callback(uint gpio, uint32_t events) {
  if(!do_render && !st7789::dma_is_busy()) {
    st7789::update();
    do_render = true;
  }
}

void init_display() {
  st7789::frame_buffer = screen_fb;
  st7789::init();
  st7789::clear();

  have_vsync = st7789::vsync_callback(vsync_callback);
}

void update_display(uint32_t time) {
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
}

void init_display_core1() {
}

void update_display_core1() {
}

bool display_render_needed() {
  return do_render;
}

void display_mode_changed(blit::ScreenMode new_mode) {
  if(have_vsync)
    do_render = true; // prevent starting an update during switch

  st7789::set_pixel_double(new_mode == ScreenMode::lores);

  if(new_mode == ScreenMode::hires)
    st7789::frame_buffer = screen_fb;
}

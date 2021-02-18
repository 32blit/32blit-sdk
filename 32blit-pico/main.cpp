#include <stdio.h>

#include "pico/stdlib.h"

#include "st7789.hpp"

#include "engine/api_private.hpp"
#include "engine/engine.hpp"
#include "graphics/surface.hpp"

using namespace blit;

uint8_t screen_fb[240 * 240 * 2];
static Surface lores_screen(screen_fb, PixelFormat::RGB, Size(160, 120));
static Surface hires_screen(screen_fb, PixelFormat::RGB565, Size(240, 240));
//static Surface hires_palette_screen(screen_fb, PixelFormat::P, Size(320, 240));

pimoroni::ST7789 st7789(240, 240, (uint16_t *)screen_fb);

ScreenMode cur_screen_mode = ScreenMode::lores;

static Surface &set_screen_mode(ScreenMode mode) {
  switch(mode) {
    case ScreenMode::lores:
      blit::screen = lores_screen;
      // window
      st7789.set_window(40, 60, 160, 120);
      st7789.set_bytes_per_pixel(3);
      break;

    case ScreenMode::hires:
      screen = hires_screen;
      st7789.set_window(0, 0, 240, 240);
      st7789.set_bytes_per_pixel(2);
      break;

    //case ScreenMode::hires_palette:
    //  screen = hires_palette_screen;
    //  break;
  }

  cur_screen_mode = mode;

  return blit::screen;
}

static uint32_t now() {
  return to_ms_since_boot(get_absolute_time());
}

// user funcs
void init();
void render(uint32_t);
void update(uint32_t);

int main() {
  stdio_init_all();

  api.now = ::now;
  api.set_screen_mode = ::set_screen_mode;

  st7789.init();
  st7789.clear();

  ::set_screen_mode(ScreenMode::lores);

  blit::render = ::render;
  blit::update = ::update;

  // user init
  ::init();

  uint32_t last_render = 0;

  while(true) {
    tick(::now());

    auto now = ::now();

    if(now - last_render >= 20 && !st7789.dma_is_busy()) {
      ::render(now);
      st7789.update();
      last_render = now;
    }
  }

  return 0;
}

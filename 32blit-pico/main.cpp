#include <stdio.h>

#include "pico/stdlib.h"

#include "st7789.hpp"

#include "engine/api_private.hpp"
#include "engine/engine.hpp"
#include "graphics/surface.hpp"

using namespace blit;

pimoroni::ST7789 st7789(240, 240, nullptr);
using ST7789Reg = pimoroni::ST7789::reg;

uint8_t screen_fb[160 * 120 * 3];
static Surface lores_screen(screen_fb, PixelFormat::RGB, Size(160, 120));
//static Surface hires_screen(screen_fb, PixelFormat::RGB, Size(320, 240));
//static Surface hires_palette_screen(screen_fb, PixelFormat::P, Size(320, 240));

ScreenMode cur_screen_mode = ScreenMode::lores;

static Surface &set_screen_mode(ScreenMode mode) {
  switch(mode) {
    case ScreenMode::lores:
      blit::screen = lores_screen;
      // window
      st7789.command(ST7789Reg::CASET, 4, "\x00\x28\x00\xc7"); // 40 - 199
      st7789.command(ST7789Reg::RASET, 4, "\x00\x3C\x00\xb3"); // 60 - 179
      break;

    //case ScreenMode::hires:
    //  screen = hires_screen;
    //  break;

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

static void update_display() {
  auto start = get_absolute_time();

  st7789.command(ST7789Reg::RAMWR, 160 * 120 * 3, (const char*)screen_fb);
  auto end = get_absolute_time();

  int elapsed = absolute_time_diff_us(start, end);
  printf("%i\n", elapsed);
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
  //
  st7789.command(ST7789Reg::COLMOD, 1, "\x06");

  ::set_screen_mode(ScreenMode::lores);

  blit::render = ::render;
  blit::update = ::update;

  // user init
  ::init();

  uint32_t last_render = 0;

  while(true) {
    tick(::now());

    auto now = ::now();

    if(now - last_render >= 20) {
      ::render(now);
      update_display();
      last_render = now;
    }
  }

  return 0;
}
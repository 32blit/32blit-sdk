
#include <string>
#include <cstring>
#include <memory>
#include <cstdlib>


#include "rotozoom.hpp"

using namespace blit;

uint8_t logo[16][16] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 0, 0, 0, 0, 1, 0, 2, 0, 1, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 2, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0},
  {0, 0, 2, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0},
  {0, 0, 2, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;

uint8_t mask_buffer[320 * 240];
Surface hires_mask(mask_buffer, PixelFormat::M, Size(320, 240));
Surface lores_mask(mask_buffer, PixelFormat::M, Size(160, 120));
Surface mask = hires_mask;
SpriteSheet *ss;
/* setup */
void init() {
  set_screen_mode(hires);

  ss = SpriteSheet::load(packed_data);
  screen.sprites = ss;
}

void rotozoom(uint32_t time_ms) {
  static float angle = 0.0f;

  static Pen palette[] = { Pen(0, 0, 0), Pen(255, 255, 255), Pen(0, 255, 0) };

  int32_t c = cos(angle * pi / 180.0f) * 1024;
  int32_t s = sin(angle * pi / 180.0f) * 1024;

  angle += 0.25f;
  angle = angle >= 360.0f ? 0.0f : angle;

  Point p;

  int16_t w = screen.bounds.w;
  int16_t h = screen.bounds.h;

  int16_t hw = w / 2;
  int16_t hh = h / 2;

  uint32_t o = 0;
  for (p.y = 0; p.y < h; p.y++) {
    for (p.x = 0; p.x < w; p.x++) {
      uint8_t u = ((((p.x - hw) * c - (p.y - hh) * s) * s) >> 20) & 0b1111;
      uint8_t v = ((((p.x - hw) * s + (p.y - hh) * c) * s) >> 20) & 0b1111;

      uint8_t pi = logo[15 - u][v];

      // slower to call the pixel routine due to extra overhead of the function 
      // call and clipping      
      // - screen.pen(palette[pi]);
      // - screen.pixel(p);

      // however we know our coordinates are within bounds so we can call the
      // pen blend function (pbf) directly for a big speed up!
      screen.pbf(&palette[pi], &screen, o, 1);
      
      o++;
    }
  }
}

int tick_count = 0;
void render(uint32_t time_ms) {
  screen.alpha = 255;
  screen.mask = nullptr;
  screen.pen = Pen(0, 0, 0, 255);
  screen.clear();
  
  mask.pen = Pen(50);
  mask.clear();
  mask.pen = Pen(255);
  Point centre = Point(160 + sin(time_ms / 200.0f) * 40, 120 + cos(time_ms / 200.0f) * 40);
  mask.circle(centre, 100);

  //screen.mask = &mask;

  uint32_t ms_start = now();

  rotozoom(time_ms);
  
  uint32_t ms_end = now();  
/*
  for (auto y = 0; y < 10; y++) {
    for (auto x = 0; x < 10; x++) {
      float s = (sin(time_ms / 1000.0f) * 2.0f) + 3.0f;
      screen.sprite(Point(x, y), Point(x * 8 * s + 160, y * 8 * s + 120), Point(40, 40), Vec2(s, s));
    }
  }*/

  screen.mask = nullptr;
  

  // draw FPS meter
  screen.pen = Pen(0, 0, 0, 200);
  screen.rectangle(Rect(5, 5, 20, 16));
  screen.pen = Pen(255, 0, 0);
  std::string fms = std::to_string(ms_end - ms_start);
  screen.text(fms, minimal_font, Rect(10, 10, 10, 16));

  int block_size = 4;
  for (uint32_t i = 0; i < (ms_end - ms_start); i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(Rect(i * (block_size + 1) + 1, screen.bounds.h - block_size - 1, block_size, block_size));
  }

}

void update(uint32_t time) {
  static uint32_t last_buttons = 0;

  if (buttons != last_buttons) {  
    if ((buttons & DPAD_UP)) {
      set_screen_mode(lores);
      mask = lores_mask;
      screen.sprites = ss;
    }
    else {
      set_screen_mode(hires);
      mask = hires_mask;
      screen.sprites = ss;
    }
  }

  last_buttons = buttons;
}

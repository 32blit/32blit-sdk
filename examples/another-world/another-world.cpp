/* 
  Another World game engine demo

  by Jonathan Williamson (lowfatcode)
*/

#include <cstring>

#include "32blit.hpp"
#include "virtual-machine.hpp"

using namespace blit;
using namespace another_world;

VirtualMachine vm;

Pen palette_buffer[16];

void init() {
  set_screen_mode(ScreenMode::hires);

  another_world::read_file = [](std::string filename, uint32_t offset, uint32_t length, char* buffer) {
    blit::File file(filename);
    uint32_t bytes_read = file.read(offset, length, buffer);
    return bytes_read == length;
  };
  
  another_world::update_screen = [](uint8_t *buffer) {
  };  

  another_world::set_palette = [](uint16_t* p) {
    // sixten palette entries in the format 0x0RGB
    for (uint8_t i = 0; i < 16; i++) {
      uint16_t color = p[i];

      uint8_t r = (color & 0b0000000000001111) >> 0;
      uint8_t g = (color & 0b1111000000000000) >> 12;
      uint8_t b = (color & 0b0000111100000000) >> 8;
      palette_buffer[i].r = (r << 4) | r;
      palette_buffer[i].g = (g << 4) | g;
      palette_buffer[i].b = (b << 4) | b; 
      palette_buffer[i].a = 255;
    }
  };

  vm.init();
  vm.initialise_chapter(16001);
}


void render(uint32_t time_ms) {
  static uint16_t tick = 0;
  tick++;

  uint32_t ms_start = now();

  screen.pen = Pen(0, 0, 0);
  screen.clear();  

  for(uint16_t y = 0; y < 200; y++) {
    for(uint16_t x = 0; x < 160; x++) {
      uint8_t v = vm.visible_vram[y * 160 + x];
      uint8_t v1 = v >> 4;
      uint8_t v2 = v & 0x0f;

      screen.pen = palette_buffer[v1];
      screen.pixel(blit::Point(x * 2, y + 20));
      screen.pen = palette_buffer[v2];
      screen.pixel(blit::Point(x * 2 + 1, y + 20));
    }
  }  

  uint32_t ms_end = now();  

  // draw FPS meter & watermark
  screen.watermark();
  screen.pen = Pen(255, 255, 255);
  screen.text(std::to_string(ms_end - ms_start) + "ms/frame", minimal_font, blit::Point(2, 240 - 10));
  screen.pen = Pen(255, 0, 0);
  for (int i = 0; i < uint16_t(ms_end - ms_start); i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(blit::Rect(i * 3 + 2, 227, 2, 2));
  }  
}

void update(uint32_t time_ms) {
  static uint32_t last_frame_clock = 0;

  uint16_t pause_ms = 20 * vm.registers[0xff];
  if(now() - last_frame_clock > pause_ms) {
    last_frame_clock = now();
    vm.execute_threads();
  }  

  another_world::input.up = buttons & DPAD_UP;
  another_world::input.down = buttons & DPAD_DOWN;
  another_world::input.left = buttons & DPAD_LEFT;
  another_world::input.right  = buttons & DPAD_RIGHT;
  another_world::input.action = buttons & A;
}

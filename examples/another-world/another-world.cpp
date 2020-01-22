/* 
  32blit Voxel Terrain demo

  by Jonathan Williamson (lowfatcode)
*/

#include <string.h>

#include "virtual-machine.hpp"
#include "another-world.hpp"

using namespace blit;
using namespace another_world;

VirtualMachine vm;

rgba palette_buffer[16];

void init() {
  another_world::read_file = [](std::string filename, uint32_t offset, uint32_t length, char* buffer) {
    uint32_t fh = blit::open_file(filename);
    uint32_t bytes_read = blit::read_file(fh, offset, length, buffer);
    blit::close_file(fh);
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
  vm.initialise_chapter(16007);
}


void render(uint32_t time_ms) {
  static uint16_t tick = 0;
  tick++;

  uint32_t ms_start = now();

  fb.pen(rgba(0, 0, 0));
  fb.clear();  

  for(uint16_t y = 0; y < 100; y++) {
    for(uint16_t x = 0; x < 160; x++) {
      uint8_t v = vm.visible_vram[(y * 2) * 160 + x];
      uint8_t v1 = v >> 4;

      fb.pen(palette_buffer[v1]);
      fb.pixel(point(x, y + 10));
    }
  }  

  uint32_t ms_end = now();  

  // draw FPS meter & watermark
  fb.watermark();
  fb.pen(rgba(255, 255, 255));
  fb.text(std::to_string(ms_end - ms_start) + "ms/frame", &minimal_font[0][0], point(1, 110));
  fb.pen(rgba(255, 0, 0));
  for (int i = 0; i < uint16_t(ms_end - ms_start); i++) {
    fb.pen(rgba(i * 5, 255 - (i * 5), 0));
    fb.rectangle(rect(i * 3 + 1, 117, 2, 2));
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

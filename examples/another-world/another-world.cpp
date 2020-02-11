/* 
  Another World demo

  by Jonathan Williamson (lowfatcode)
*/

#include <string.h>
#include <stdint.h>

#include "32blit.hpp"
#include "virtual-machine.hpp"

using namespace blit;
using namespace another_world;

int load_count = 19;

VirtualMachine vm;

void init() {
  set_screen_mode(ScreenMode::lores);
  init_resources();

  vm.init();
  
  resources[19]->state = Resource::State::NEEDS_LOADING;  // main screen
  resources[20]->state = Resource::State::NEEDS_LOADING;  // main screen palette
  load_needed_resources();

}


void render(uint32_t time_ms) {
  static uint16_t tick = 0;
  tick++;

  // draw the sky
  screen.pen = Pen(20, 30, 40);
  screen.clear();  
/*

  uint8_t j = 0;
  for(auto i = 0; i < load_count; i++) {
    Resource *resource = resources[i];    
    for(auto j = 0; j < (160 < resource->size ? 160 : resource->size); j++) {
      uint8_t v = resource->data[j];
      screen.pen(rgba(v, v, v));
      screen.pixel(point(j, i + 1));
    }
  }

  
  Resource *resource = resources[18];    
  for(auto y = 0; y < 120; y++) {
    for(auto x = 0; x < 160; x++) {
      uint8_t v = resource->data[(y * 320) + x];
      screen.pen(rgba(v, v, v));
      screen.pixel(point(x, y));
    }
  }*/
  
/*
  Resource *resource = resources[18];   
  for(auto y = 0; y < 120; y++) {
    for(auto x = 0; x < 80; x ++) {
      uint8_t v = resource->data[y * 160 + x];
      uint8_t h = v >> 4;
      uint8_t l = v & 0x0f;

      h <<= 4;
      l <<= 4;

      screen.pen(rgba(h, h, h));
      screen.pixel(point(x * 2 + 0, y));
      screen.pen(rgba(l, l, l));
      screen.pixel(point(x * 2 + 1, y));

    }
  }*/ 

/*
palette decoding?

// *buf is pointer to palette resource->data i guess
// num appears to be a palette index within that buffer, the first 1024 bytes
// of the buffer contain the palettes for MSDOS
// at two bytes per colour, that's 32 bytes per palette
// therefore a palette resource can contain up to 32 different palettes

static void readPaletteAmiga(const uint8_t *buf, int num, Color pal[16]) {
	const uint8_t *p = buf + num * 16 * sizeof(uint16_t);
	for (int i = 0; i < 16; ++i) {
		const uint16_t color = READ_BE_UINT16(p); p += 2;
		const uint8_t r = (color >> 8) & 0xF;
		const uint8_t g = (color >> 4) & 0xF;
		const uint8_t b =  color       & 0xF;
		pal[i].r = (r << 4) | r;
		pal[i].g = (g << 4) | g;
		pal[i].b = (b << 4) | b;
	}
}*/
  Pen palette[16];
  for(auto i = 0; i < 16; i++) {
    uint8_t palette_idx = tick / 8;
    uint16_t po = palette_idx * 16 * sizeof(uint16_t);
    
    uint16_t p = read_uint16_bigendian(*((uint32_t *)&resources[20]->data[po + (i * sizeof(uint16_t))]));
    
    uint8_t r = (p >> 8) & 0x0f;
    uint8_t g = (p >> 4) & 0x0f;
    uint8_t b = (p >> 0) & 0x0f;

    palette[i] = Pen(
      (r << 4) | r,
      (g << 4) | g,
      (b << 4) | b
    );
  }

  for(auto y = 0; y < 160; y++) {
    for(auto x = 0; x < 40; x ++) {
      uint16_t o = (y * 40) + x;

      // get 8 pixels worth of data from the frame buffer
      uint8_t b0 = resources[19]->data[o +     0];
      uint8_t b1 = resources[19]->data[o +  8000];
      uint8_t b2 = resources[19]->data[o + 16000];
      uint8_t b3 = resources[19]->data[o + 24000];

      for(int i = 0; i < 8; i++) {
        uint8_t v = ((b3 & 0x80) >> 4) | ((b2 & 0x80) >> 5) | ((b1 & 0x80) >> 6) | ((b0 & 0x80) >> 7);

        //v *= 16;
        //screen.pen(rgba(v, v, v));
        screen.pen = palette[v];
        
        screen.pixel(blit::Point((x * 8 + i), y));

        b0 <<= 1; b1 <<= 1; b2 <<= 1; b3 <<= 1;
      }
    }
  }
  
  for(auto palette_idx = 0; palette_idx < 32; palette_idx++) {
    for(auto i = 0; i < 16; i++) {
      uint16_t po = palette_idx * 16 * sizeof(uint16_t);
      
      uint16_t p = read_uint16_bigendian(*((uint32_t *)&resources[20]->data[po + (i * sizeof(uint16_t))]));
      
      uint8_t r = (p >> 8) & 0x0f;
      uint8_t g = (p >> 4) & 0x0f;
      uint8_t b = (p >> 0) & 0x0f;

      Pen col = Pen(
        (r << 4) | r,
        (g << 4) | g,
        (b << 4) | b
      );

      screen.pen = col;
      screen.pixel(blit::Point(i, palette_idx));
    }
  }
/*
  return;

  if(tick % 20 == 0) {
    vm.execute_threads();
  }
  
  
  screen.pen = Pen(255, 255, 255);

  for(auto i = 0; i < 10; i++) {
    for(auto j = 0; j < 5; j++) {
      std::string v = std::to_string(vm.registers[j + (i * 10)]);
      screen.text(v, &minimal_font[0][0], blit::Point(j * 30, i * 10 + 30));  
    }
  }

  for(auto j = 0; j < 8; j++) {
    std::string v = std::to_string(vm.program_counter[j]);

    if(vm.program_counter[j] == 0xffff) {
      v = "--";
    }

    screen.text(v, &minimal_font[0][0], blit::Point(j * 20, 0));  

    std::string v2 = std::to_string(resource_heap[vm.program_counter[j]]);

    screen.text(v2, &minimal_font[0][0], blit::Point(j * 20, 10));  
  }
  
  //screen.text(std::to_string(resources[18]->size), &minimal_font[0][0], point(20, 100));

  uint32_t ms_start = now();
  screen.pen = Pen(255, 255, 255);
  screen.pixel(blit::Point(tick % 160, 0));
  uint32_t ms_end = now();  

  // draw FPS meter & watermark
  screen.watermark();
  screen.text(std::to_string(ms_end - ms_start), &minimal_font[0][0], blit::Point(1, 110));
  screen.pen = Pen(255, 0, 0);
  for (int i = 0; i < uint16_t(ms_end - ms_start); i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(blit::Rect(i * 3 + 1, 117, 2, 2));
  }  */
}

void update(uint32_t time_ms) {
  static uint16_t tick = 0;
  tick++;
}
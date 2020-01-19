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

bool aread_file(std::string filename, uint32_t offset, uint32_t length, char* buffer) {
  uint32_t fh = blit::open_file(filename);
  uint32_t bytes_read = blit::read_file(fh, offset, length, buffer);
  blit::close_file(fh);
  return bytes_read == length;
};

void adebug_log(const char *fmt, ...) {
  /*static bool first = true; // if first run then open the file and truncate
  va_list args;

  std::wstring filename = L"vm.log";
  va_start(args, fmt);
  std::string line = string_format(fmt, args) + "\n";
  va_end(args);
  DWORD creation_mode = first ? CREATE_ALWAYS : OPEN_EXISTING;
  HANDLE fh = CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, creation_mode, FILE_ATTRIBUTE_NORMAL, NULL);
  SetFilePointer(fh, 0, NULL, FILE_END);
  
  WriteFile(fh, line.c_str(), line.length(), NULL, NULL);

  CloseHandle(fh);

  first = false;*/
};

void adebug(const char *fmt, ...) {
 /* va_list args;
  va_start(args, fmt);
  std::string line = string_format(fmt, args) + "\n";
  va_end(args);

  std::wstring w(line.begin(), line.end());
  OutputDebugStringW(w.c_str());*/
};

//uint8_t sbuf[160 * 200];

bool updated = false;
void aupdate_screen(uint8_t *buffer) {
  //memcpy(sbuf, buffer, 160 * 200);
  
};

rgba pbuf[16];
void aset_palette(uint16_t* p) {
  // sixten palette entries in the format 0x0RGB
  for (uint8_t i = 0; i < 16; i++) {
    uint16_t color = p[i];

    uint8_t r = (color & 0b0000000000001111) >> 0;
    uint8_t g = (color & 0b1111000000000000) >> 12;
    uint8_t b = (color & 0b0000111100000000) >> 8;
    pbuf[i].r = (r << 4) | r;
    pbuf[i].g = (g << 4) | g;
    pbuf[i].b = (b << 4) | b; 
    pbuf[i].a = 255;
  }
};

void adebug_yield() {
 /* InvalidateRect(hWnd, NULL, FALSE);
  UpdateWindow(hWnd);

  Sleep(10);*/
};

void init() {
  another_world::read_file = ::aread_file;
  another_world::debug_log = adebug_log;
  another_world::debug = adebug;  
  another_world::update_screen = aupdate_screen;  
  another_world::set_palette = aset_palette;  
  another_world::debug_yield = adebug_yield;  
    load_resource_list();
    vm.init();

    // initialise the first chapter
    vm.initialise_chapter(16001);
}


void render(uint32_t time_ms) {
  static uint16_t tick = 0;
  tick++;

  uint32_t ms_start = now();

  if(tick & 0b1) {
    vm.execute_threads();
  }

  fb.pen(rgba(0, 0, 0));
  fb.clear();  

  for(uint16_t y = 0; y < 100; y++) {
    for(uint16_t x = 0; x < 160; x++) {
      uint8_t v = vm.visible_vram[(y * 2) * 160 + x];
      uint8_t v1 = v >> 4;

      fb.pen(pbuf[v1]);
      fb.pixel(point(x, y + 10));
    }
  }  

  uint32_t ms_end = now();  

  // draw FPS meter & watermark
  fb.watermark();
  fb.text(std::to_string(ms_end - ms_start), &minimal_font[0][0], point(1, 110));
  fb.pen(rgba(255, 0, 0));
  for (int i = 0; i < uint16_t(ms_end - ms_start); i++) {
    fb.pen(rgba(i * 5, 255 - (i * 5), 0));
    fb.rectangle(rect(i * 3 + 1, 117, 2, 2));
  }  
}

void update(uint32_t time_ms) {
  static uint16_t tick = 0;
  tick++;
}

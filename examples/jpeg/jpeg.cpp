
#include <cstring>

#include "jpeg.hpp"
#include "graphics/jpeg.hpp"

#include "assets.hpp"

using namespace blit;

/* setup */
void init() {
  set_screen_mode(ScreenMode::hires);

  // It's also possible to use decode_jpeg_file to load a file from the SD card
  JPEGImage jpeg = decode_jpeg_buffer(asset_jpeg, asset_jpeg_length);

  //Surface jpeg_surface(jpeg.data, PixelFormat::RGB, jpeg.size);
  //screen.blit(&jpeg_surface, Rect(Point(0), jpeg.size), Point(0));

  // Display fullscreen jpeg
  memcpy(screen.data, jpeg.data, jpeg.size.area() * 3);

  delete[] jpeg.data;
}

void render(uint32_t time) {
}

void update(uint32_t time) {
}
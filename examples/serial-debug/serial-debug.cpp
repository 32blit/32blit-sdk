#include <stdint.h>

#include "serial-debug.hpp"
#include "graphics/color.hpp"

using namespace blit;

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 120
#define COL1 5
#define COL2 55
#define COL3 105

#define ROW1 38
#define ROW2 70
#define ROW2_5 86
#define ROW3 102


void init() {
    set_screen_mode(screen_mode::lores);
}


void render(uint32_t time) {
  char text_buf[100] = {0};

  for(int b = 0; b < SCREEN_WIDTH; b++){
    for(int v = 0; v < SCREEN_HEIGHT; v++){
        fb.pen(blit::hsv_to_rgba(float(b) / (float)(SCREEN_WIDTH), 1.0f, float(v) / (float)(SCREEN_HEIGHT)));
        fb.pixel(point(b, v));
    }
  }

  fb.text("Time:", &minimal_font[0][0], point(COL1, ROW1));

  sprintf(text_buf, "%lu", time);
  fb.text(text_buf, &minimal_font[0][0], point(COL2, ROW1));

  blit::debugf("Hello from 32blit time = %lu\n\r", time);

}

void update(uint32_t time) {

}




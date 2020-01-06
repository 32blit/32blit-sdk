#include <stdint.h>

#include "32blit.hpp"
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
}

void update(uint32_t time) {

}




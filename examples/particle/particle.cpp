// Tearing test

#include "32blit.hpp"

uint16_t width = 160;
uint16_t height = 120;
uint32_t prev_buttons = blit::buttons;

uint32_t updates = 0;

void init(void) {
    blit::set_screen_mode(blit::lores);
}

void update(uint32_t time) {
    updates++;
}


void render(uint32_t time) {
    static uint32_t renders = 0;

    if (blit::buttons) {
        blit::fb.pen(blit::rgba(127, 127, 127));
        blit::fb.clear();
    } else {
        blit::fb.pen(blit::rgba(0, 0, 0));
        blit::fb.clear();
        if (renders&1) {
            blit::fb.pen(blit::rgba(255, 255, 255));
            blit::fb.rectangle(blit::rect(70, 50, 20, 20));
        }
    }
    renders++;
}
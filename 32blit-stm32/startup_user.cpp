#include "engine/engine.hpp"

extern void init();
extern void update(uint32_t time);
extern void render(uint32_t time);

extern "C" void cpp_do_init() {

    blit::update = update;
    blit::render = render;

    blit::set_screen_mode(blit::ScreenMode::lores);

    init();
}
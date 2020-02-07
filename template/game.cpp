#include "game.hpp"

using namespace blit;

///////////////////////////////////////////////////////////////////////////
//
// init()
//
// setup your game here
//
void init() {
    set_screen_mode(screen_mode::hires);
}

///////////////////////////////////////////////////////////////////////////
//
// render(time)
//
// This function is called to perform rendering of the game. time is the 
// amount if milliseconds elapsed since the start of your game
//
void render(uint32_t time) {

    // clear the screen -- fb is a reference to the frame buffer and can be used to draw all things with the 32blit
    fb.clear();

    // draw some text at the top of the screen
    fb.alpha = 255;
    fb.mask = nullptr;
    fb.pen(rgba(255, 255, 255));
    fb.rectangle(rect(0, 0, 320, 14));
    fb.pen(rgba(0, 0, 0));
    fb.text("Hello 32blit!", &minimal_font[0][0], point(5, 4));
}

///////////////////////////////////////////////////////////////////////////
//
// update(time)
//
// This is called to update your game state. time is the 
// amount if milliseconds elapsed since the start of your game
//
void update(uint32_t time) {
}
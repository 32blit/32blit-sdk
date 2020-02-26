#include "hardware-test.hpp"
#include "graphics/color.hpp"
#include <cmath>

using namespace blit;

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 120

void init() {
    set_screen_mode(ScreenMode::lores);
}

#define COL1 5
#define COL2 55
#define COL3 105

#define ROW1 38
#define ROW2 70
#define ROW3 102

void render(uint32_t time) {
    char text_buf[100] = {0};
    bool button_a = blit::buttons & blit::Button::A;
    bool button_b = blit::buttons & blit::Button::B;
    bool button_x = blit::buttons & blit::Button::X;
    bool button_y = blit::buttons & blit::Button::Y;
    bool button_m = blit::buttons & blit::Button::MENU;
    bool button_h = blit::buttons & blit::Button::HOME;
    bool dpad_l = blit::buttons & blit::Button::DPAD_LEFT;
    bool dpad_r = blit::buttons & blit::Button::DPAD_RIGHT;
    bool dpad_u = blit::buttons & blit::Button::DPAD_UP;
    bool dpad_d = blit::buttons & blit::Button::DPAD_DOWN;

    for(int b = 0; b < SCREEN_WIDTH; b++){
        for(int v = 0; v < SCREEN_HEIGHT; v++){
            screen.pen = blit::hsv_to_rgba(float(b) / (float)(SCREEN_WIDTH), 1.0f, float(v) / (float)(SCREEN_HEIGHT));
            screen.pixel(Point(b, v));
        }
    }

    vibration = 0.0f;
    if(button_a){vibration += 0.1f;}
    if(button_b){vibration += 0.2f;}
    if(button_x){vibration += 0.3f;}
    if(button_y){vibration += 0.4f;}

    screen.pen = button_a ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("A", minimal_font, Point(150, 15));

    screen.pen = button_b ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("B", minimal_font, Point(140, 25));

    screen.pen = button_x ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("X", minimal_font, Point(140, 5));

    screen.pen = button_y ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("Y", minimal_font, Point(130, 15));


    screen.pen = dpad_r ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("R", minimal_font, Point(25, 15));

    screen.pen = dpad_d ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("D", minimal_font, Point(15, 25));

    screen.pen = dpad_u ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("U", minimal_font, Point(15, 5));

    screen.pen = dpad_l ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("L", minimal_font, Point(5, 15));


    screen.pen = button_m ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("MENU", minimal_font, Point(55, 15));

    screen.pen = button_h ? Pen(255, 0, 0) : Pen(128, 128, 128);
    screen.text("HOME", minimal_font, Point(85, 15));


    screen.pen = Pen(255, 0, 0);

    screen.pixel(Point(
        (SCREEN_WIDTH / 2) + blit::joystick.x * 30,
        (SCREEN_HEIGHT / 2) + blit::joystick.y * 30
    ));


    screen.pen = Pen(255, 255, 255);

    screen.text("Joystick:", minimal_font, Point(COL1, ROW1));

    snprintf(text_buf, 100, "X: %d", (int)(blit::joystick.x * 1024));
    screen.text(text_buf, minimal_font, Point(COL1, ROW1+7));

    snprintf(text_buf, 100, "Y: %d", (int)(blit::joystick.y * 1024));
    screen.text(text_buf, minimal_font, Point(COL1, ROW1+14));

    screen.text("B:", minimal_font, Point(COL1, ROW1+21));
    if(blit::buttons & blit::Button::JOYSTICK) {
        screen.rectangle(Rect(
            COL1+10, ROW1+22,
            5, 5
        ));
    }

    screen.text("Tilt:", minimal_font, Point(COL2, ROW1));

    snprintf(text_buf, 100, "X: %d", (int)(blit::tilt.x * 1024));
    screen.text(text_buf, minimal_font, Point(COL2, ROW1+7));

    snprintf(text_buf, 100, "Y: %d", (int)(blit::tilt.y * 1024));
    screen.text(text_buf, minimal_font, Point(COL2, ROW1+14));

    snprintf(text_buf, 100, "Z: %d", (int)(blit::tilt.z * 1024));
    screen.text(text_buf, minimal_font, Point(COL2, ROW1+21));

    blit::LED = Pen(
        (float)((sin(blit::now() / 100.0f) + 1) / 2.0f),
        (float)((cos(blit::now() / 100.0f) + 1) / 2.0f),
        (float)((sin(blit::now() / 100.0f) + 1) / 2.0f)
    );
}

void update(uint32_t time) {

}

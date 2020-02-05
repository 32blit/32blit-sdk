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
            screen.pen(blit::hsv_to_rgba(float(b) / (float)(SCREEN_WIDTH), 1.0f, float(v) / (float)(SCREEN_HEIGHT)));
            screen.pixel(Point(b, v));
        }
    }

    vibration = 0.0f;
    if(button_a){vibration += 0.1f;}
    if(button_b){vibration += 0.2f;}
    if(button_x){vibration += 0.3f;}
    if(button_y){vibration += 0.4f;}

    screen.pen(button_a ? RGBA(255, 0, 0) : RGBA(128, 128, 128));
    screen.text("A", &minimal_font[0][0], Point(150, 15));

    screen.pen(button_b ? RGBA(255, 0, 0) : RGBA(128, 128, 128));
    screen.text("B", &minimal_font[0][0], Point(140, 25));

    screen.pen(button_x ? RGBA(255, 0, 0) : RGBA(128, 128, 128));
    screen.text("X", &minimal_font[0][0], Point(140, 5));

    screen.pen(button_y ? RGBA(255, 0, 0) : RGBA(128, 128, 128));
    screen.text("Y", &minimal_font[0][0], Point(130, 15));


    screen.pen(dpad_r ? RGBA(255, 0, 0) : RGBA(128, 128, 128));
    screen.text("R", &minimal_font[0][0], Point(25, 15));

    screen.pen(dpad_d ? RGBA(255, 0, 0) : RGBA(128, 128, 128));
    screen.text("D", &minimal_font[0][0], Point(15, 25));

    screen.pen(dpad_u ? RGBA(255, 0, 0) : RGBA(128, 128, 128));
    screen.text("U", &minimal_font[0][0], Point(15, 5));

    screen.pen(dpad_l ? RGBA(255, 0, 0) : RGBA(128, 128, 128));
    screen.text("L", &minimal_font[0][0], Point(5, 15));


    screen.pen(button_m ? RGBA(255, 0, 0) : RGBA(128, 128, 128));
    screen.text("MENU", &minimal_font[0][0], Point(55, 15));

    screen.pen(button_h ? RGBA(255, 0, 0) : RGBA(128, 128, 128));
    screen.text("HOME", &minimal_font[0][0], Point(85, 15));


    screen.pen(RGBA(255, 0, 0));

    screen.pixel(Point(
        (SCREEN_WIDTH / 2) + blit::joystick.x * 30,
        (SCREEN_HEIGHT / 2) + blit::joystick.y * 30
    ));


    screen.pen(RGBA(255, 255, 255));

    screen.text("Joystick:", &minimal_font[0][0], Point(COL1, ROW1));

    sprintf(text_buf, "X: %d", (int)(blit::joystick.x * 1024));
    screen.text(text_buf, &minimal_font[0][0], Point(COL1, ROW1+7));

    sprintf(text_buf, "Y: %d", (int)(blit::joystick.y * 1024));
    screen.text(text_buf, &minimal_font[0][0], Point(COL1, ROW1+14));

    screen.text("B:", &minimal_font[0][0], Point(COL1, ROW1+21));
    if(blit::buttons & blit::Button::JOYSTICK) {
        screen.rectangle(Rect(
            COL1+10, ROW1+22,
            5, 5
        ));
    }

    screen.text("Tilt:", &minimal_font[0][0], Point(COL2, ROW1));

    sprintf(text_buf, "X: %d", (int)(blit::tilt.x * 1024));
    screen.text(text_buf, &minimal_font[0][0], Point(COL2, ROW1+7));

    sprintf(text_buf, "Y: %d", (int)(blit::tilt.y * 1024));
    screen.text(text_buf, &minimal_font[0][0], Point(COL2, ROW1+14));

    sprintf(text_buf, "Z: %d", (int)(blit::tilt.z * 1024));
    screen.text(text_buf, &minimal_font[0][0], Point(COL2, ROW1+21));

    blit::LED = RGB(
        (float)((sin(blit::now() / 100.0f) + 1) / 2.0f),
        (float)((cos(blit::now() / 100.0f) + 1) / 2.0f),
        (float)((sin(blit::now() / 100.0f) + 1) / 2.0f)
    );

    screen.text("Bat VBUS:", &minimal_font[0][0], Point(COL1, ROW3));
    switch(battery_vbus_status){
        case 0b00: // Unknown
            screen.text("Unknown", &minimal_font[0][0], Point(COL1, ROW3+7));
            break;
        case 0b01: // USB Host
            screen.text("USB Host", &minimal_font[0][0], Point(COL1, ROW3+7));
            break;
        case 0b10: // Adapter Port
            screen.text("Adapter", &minimal_font[0][0], Point(COL1, ROW3+7));
            break;
        case 0b11: // OTG
            screen.text("OTG", &minimal_font[0][0], Point(COL1, ROW3+7));
            break;
    }

    screen.text("Bat Chrg:", &minimal_font[0][0], Point(COL2, ROW3));
    switch(battery_charge_status){
        case 0b00: // Not Charging
            screen.text("Nope", &minimal_font[0][0], Point(COL2, ROW3+7));
            break;
        case 0b01: // Pre-charge
            screen.text("Pre", &minimal_font[0][0], Point(COL2, ROW3+7));
            break;
        case 0b10: // Fast Charging
            screen.text("Fast", &minimal_font[0][0], Point(COL2, ROW3+7));
            break;
        case 0b11: // Charge Done
            screen.text("Done", &minimal_font[0][0], Point(COL2, ROW3+7));
            break;
    }

    sprintf(text_buf, "%d", (int)(blit::battery * 1000.f));
    screen.text("Battery:", &minimal_font[0][0], Point(COL3, ROW3));
    screen.text(text_buf, &minimal_font[0][0], Point(COL3, ROW3+7));

    sprintf(text_buf, "%d", blit::battery_fault);
    screen.text("Fault:", &minimal_font[0][0], Point(COL3, ROW1));
    screen.text(text_buf, &minimal_font[0][0], Point(COL3, ROW1+7));
}

void update(uint32_t time) {

}

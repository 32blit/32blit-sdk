#include "hardware-test.hpp"
#include "graphics/color.hpp"
#include <cmath>

using namespace blit;

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 120

void init() {
    set_screen_mode(screen_mode::lores);
}

#define COL1 5
#define COL2 55
#define COL3 105

#define ROW1 38
#define ROW2 70
#define ROW3 102

void render(uint32_t time) {
    char text_buf[100] = {0};
    bool button_a = blit::buttons & blit::button::A;
    bool button_b = blit::buttons & blit::button::B;
    bool button_x = blit::buttons & blit::button::X;
    bool button_y = blit::buttons & blit::button::Y;
    bool button_m = blit::buttons & blit::button::MENU;
    bool button_h = blit::buttons & blit::button::HOME;
    bool dpad_l = blit::buttons & blit::button::DPAD_LEFT;
    bool dpad_r = blit::buttons & blit::button::DPAD_RIGHT;
    bool dpad_u = blit::buttons & blit::button::DPAD_UP;
    bool dpad_d = blit::buttons & blit::button::DPAD_DOWN;

    for(int b = 0; b < SCREEN_WIDTH; b++){
        for(int v = 0; v < SCREEN_HEIGHT; v++){
            fb.pen(blit::hsv_to_rgba(float(b) / (float)(SCREEN_WIDTH), 1.0f, float(v) / (float)(SCREEN_HEIGHT)));
            fb.pixel(point(b, v));
        }
    }

    vibration = 0.0f;
    if(button_a){vibration += 0.1f;}
    if(button_b){vibration += 0.2f;}
    if(button_x){vibration += 0.3f;}
    if(button_y){vibration += 0.4f;}

    fb.pen(button_a ? rgba(255, 0, 0) : rgba(128, 128, 128));
    fb.text("A", &minimal_font[0][0], point(150, 15));

    fb.pen(button_b ? rgba(255, 0, 0) : rgba(128, 128, 128));
    fb.text("B", &minimal_font[0][0], point(140, 25));

    fb.pen(button_x ? rgba(255, 0, 0) : rgba(128, 128, 128));
    fb.text("X", &minimal_font[0][0], point(140, 5));

    fb.pen(button_y ? rgba(255, 0, 0) : rgba(128, 128, 128));
    fb.text("Y", &minimal_font[0][0], point(130, 15));


    fb.pen(dpad_r ? rgba(255, 0, 0) : rgba(128, 128, 128));
    fb.text("R", &minimal_font[0][0], point(25, 15));

    fb.pen(dpad_d ? rgba(255, 0, 0) : rgba(128, 128, 128));
    fb.text("D", &minimal_font[0][0], point(15, 25));

    fb.pen(dpad_u ? rgba(255, 0, 0) : rgba(128, 128, 128));
    fb.text("U", &minimal_font[0][0], point(15, 5));

    fb.pen(dpad_l ? rgba(255, 0, 0) : rgba(128, 128, 128));
    fb.text("L", &minimal_font[0][0], point(5, 15));


    fb.pen(button_m ? rgba(255, 0, 0) : rgba(128, 128, 128));
    fb.text("MENU", &minimal_font[0][0], point(55, 15));

    fb.pen(button_h ? rgba(255, 0, 0) : rgba(128, 128, 128));
    fb.text("HOME", &minimal_font[0][0], point(85, 15));


    fb.pen(rgba(255, 0, 0));

    fb.pixel(point(
        (SCREEN_WIDTH / 2) + blit::joystick.x * 30,
        (SCREEN_HEIGHT / 2) + blit::joystick.y * 30
    ));


    fb.pen(rgba(255, 255, 255));

    fb.text("Joystick:", &minimal_font[0][0], point(COL1, ROW1));

    sprintf(text_buf, "X: %d", (int)(blit::joystick.x * 1024));
    fb.text(text_buf, &minimal_font[0][0], point(COL1, ROW1+7));

    sprintf(text_buf, "Y: %d", (int)(blit::joystick.y * 1024));
    fb.text(text_buf, &minimal_font[0][0], point(COL1, ROW1+14));

    fb.text("B:", &minimal_font[0][0], point(COL1, ROW1+21));
    if(blit::buttons & blit::button::JOYSTICK) {
        fb.rectangle(rect(
            COL1+10, ROW1+22,
            5, 5
        ));
    }

    fb.text("Tilt:", &minimal_font[0][0], point(COL2, ROW1));

    sprintf(text_buf, "X: %d", (int)(blit::tilt.x * 1024));
    fb.text(text_buf, &minimal_font[0][0], point(COL2, ROW1+7));

    sprintf(text_buf, "Y: %d", (int)(blit::tilt.y * 1024));
    fb.text(text_buf, &minimal_font[0][0], point(COL2, ROW1+14));

    sprintf(text_buf, "Z: %d", (int)(blit::tilt.z * 1024));
    fb.text(text_buf, &minimal_font[0][0], point(COL2, ROW1+21));

    blit::LED = rgb(
        (float)((sin(blit::now() / 100.0f) + 1) / 2.0f),
        (float)((cos(blit::now() / 100.0f) + 1) / 2.0f),
        (float)((sin(blit::now() / 100.0f) + 1) / 2.0f)
    );

    fb.text("Bat VBUS:", &minimal_font[0][0], point(COL1, ROW3));
    switch(battery_vbus_status){
        case 0b00: // Unknown
            fb.text("Unknown", &minimal_font[0][0], point(COL1, ROW3+7));
            break;
        case 0b01: // USB Host
            fb.text("USB Host", &minimal_font[0][0], point(COL1, ROW3+7));
            break;
        case 0b10: // Adapter Port
            fb.text("Adapter", &minimal_font[0][0], point(COL1, ROW3+7));
            break;
        case 0b11: // OTG
            fb.text("OTG", &minimal_font[0][0], point(COL1, ROW3+7));
            break;
    }

    fb.text("Bat Chrg:", &minimal_font[0][0], point(COL2, ROW3));
    switch(battery_charge_status){
        case 0b00: // Not Charging
            fb.text("Nope", &minimal_font[0][0], point(COL2, ROW3+7));
            break;
        case 0b01: // Pre-charge
            fb.text("Pre", &minimal_font[0][0], point(COL2, ROW3+7));
            break;
        case 0b10: // Fast Charging
            fb.text("Fast", &minimal_font[0][0], point(COL2, ROW3+7));
            break;
        case 0b11: // Charge Done
            fb.text("Done", &minimal_font[0][0], point(COL2, ROW3+7));
            break;
    }

    sprintf(text_buf, "%d", (int)(blit::battery * 1000.f));
    fb.text("Battery:", &minimal_font[0][0], point(COL3, ROW3));
    fb.text(text_buf, &minimal_font[0][0], point(COL3, ROW3+7));

    sprintf(text_buf, "%d", blit::battery_fault);
    fb.text("Fault:", &minimal_font[0][0], point(COL3, ROW1));
    fb.text(text_buf, &minimal_font[0][0], point(COL3, ROW1+7));
}

void update(uint32_t time) {

}

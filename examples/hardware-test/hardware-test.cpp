#include "hardware-test.hpp"
#include "graphics/color.hpp"

using namespace blit;

void init() {
    set_screen_mode(screen_mode::lores);
}

void render(uint32_t time) {
    fb.pen(rgba(0x4e, 0xb3, 0xf7));
    fb.clear();

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

    vibration = 0.0;
    if(button_a){vibration += 0.1;}
    if(button_b){vibration += 0.2;}
    if(button_x){vibration += 0.3;}
    if(button_y){vibration += 0.4;}

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



    fb.pen(rgba(255, 255, 255));

    char buf_joystick_x[60] = "";
    char buf_joystick_y[60] = "";

    sprintf(buf_joystick_x, "X: %d", (int)(blit::joystick.x * 1024));
    sprintf(buf_joystick_y, "Y: %d", (int)(blit::joystick.y * 1024));

    if(blit::buttons & blit::button::JOYSTICK) {
        fb.text("Joystick: (o)", &minimal_font[0][0], point(5, 80));
    }
    else
    {
        fb.text("Joystick: ( )", &minimal_font[0][0], point(5, 80));
    }

    fb.text(buf_joystick_x, &minimal_font[0][0], point(5, 80+7));
    fb.text(buf_joystick_y, &minimal_font[0][0], point(5, 80+14));

    char buf_tilt_x[10] = "";
    char buf_tilt_y[10] = "";
    char buf_tilt_z[10] = "";

    sprintf(buf_tilt_x, "X: %d", (int)(blit::tilt.x * 1024));
    sprintf(buf_tilt_y, "Y: %d", (int)(blit::tilt.y * 1024));
    sprintf(buf_tilt_z, "Z: %d", (int)(blit::tilt.z * 1024));


    for(int b = 0; b < 90; b++){
        for(int v = 0; v < 40; v++){
            fb.pen(blit::hsv_to_rgba(float(b) / (float)90.0, 1.0, (float)v / (float)40.0));
            fb.pixel(point(b + 35, 40 + v));
        }
    }

    fb.pen(rgba(255, 255, 255));
    fb.text("Tilt:", &minimal_font[0][0], point(80, 80));
    fb.text(buf_tilt_x, &minimal_font[0][0], point(80, 80+7));
    fb.text(buf_tilt_y, &minimal_font[0][0], point(80, 80+14));
    fb.text(buf_tilt_z, &minimal_font[0][0], point(80, 80+21));

    blit::LED = rgb(
        (float)((sin(blit::now() / 100.0f) + 1) / 2.0f),
        (float)((cos(blit::now() / 100.0f) + 1) / 2.0f),
        (float)((sin(blit::now() / 100.0f) + 1) / 2.0f)
    );

    if (dpad_u) {
        blit::backlight += 1.0f / 256.0f;
    }
    if (dpad_d) {
        blit::backlight -= 1.0f / 256.0f;
    }
    blit::backlight = std::fmin(1.0f, std::fmax(0.0f, blit::backlight));

    char buf_backlight[10] = "";
    sprintf(buf_backlight, "%d", (int)(blit::backlight * 1024));

    fb.pen(rgba(255, 255, 255));
    fb.text("Backlight:", &minimal_font[0][0], point(5, 80+21));
    fb.text(buf_backlight, &minimal_font[0][0], point(5, 80+28));

}

void update(uint32_t time) {

}

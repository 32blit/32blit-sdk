#include "hardware-test.hpp"
#include "graphics/color.hpp"

#include "hardware/gpio.h"

#include <cmath>

using namespace blit;

const uint8_t STATUS_V_SPACING = 10;

const uint32_t VBUS_DETECT_PIN = 2;
const uint32_t CHARGE_STATUS_PIN = 24;

const int16_t notes[2][384] = {
  { // melody notes
    147, 0, 0, 0, 0, 0, 0, 0, 175, 0, 196, 0, 220, 0, 262, 0, 247, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 175, 0, 0, 0, 0, 0, 0, 0, 175, 0, 196, 0, 220, 0, 262, 0, 330, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 349, 0, 0, 0, 0, 0, 0, 0, 349, 0, 330, 0, 294, 0, 220, 0, 262, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 247, 0, 0, 0, 0, 0, 0, 0, 247, 0, 220, 0, 196, 0, 147, 0, 175, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0,
    147, 0, 0, 0, 0, 0, 0, 0, 175, 0, 196, 0, 220, 0, 262, 0, 247, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 175, 0, 0, 0, 0, 0, 0, 0, 175, 0, 196, 0, 220, 0, 262, 0, 330, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 349, 0, 0, 0, 0, 0, 0, 0, 349, 0, 330, 0, 294, 0, 220, 0, 262, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 247, 0, 0, 0, 0, 0, 0, 0, 247, 0, 220, 0, 196, 0, 147, 0, 175, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0,
    147, 0, 0, 0, 0, 0, 0, 0, 175, 0, 196, 0, 220, 0, 262, 0, 247, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 175, 0, 0, 0, 0, 0, 0, 0, 175, 0, 196, 0, 220, 0, 262, 0, 330, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 349, 0, 0, 0, 0, 0, 0, 0, 349, 0, 330, 0, 294, 0, 220, 0, 262, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 247, 0, 0, 0, 0, 0, 0, 0, 247, 0, 262, 0, 294, 0, 392, 0, 440, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  { // rhythm notes
    294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 392, 0, 523, 0, 659, 0, 523, 0, 392, 0, 523, 0, 659, 0, 523, 0, 698, 0, 587, 0, 440, 0, 587, 0, 698, 0, 587, 0, 440, 0, 587, 0, 523, 0, 440, 0, 330, 0, 440, 0, 523, 0, 440, 0, 330, 0, 440, 0, 349, 0, 294, 0, 220, 0, 294, 0, 349, 0, 294, 0, 220, 0, 294, 0, 262, 0, 247, 0, 220, 0, 175, 0, 165, 0, 147, 0, 131, 0, 98, 0,
    294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 392, 0, 523, 0, 659, 0, 523, 0, 392, 0, 523, 0, 659, 0, 523, 0, 698, 0, 587, 0, 440, 0, 587, 0, 698, 0, 587, 0, 440, 0, 587, 0, 523, 0, 440, 0, 330, 0, 440, 0, 523, 0, 440, 0, 330, 0, 440, 0, 349, 0, 294, 0, 220, 0, 294, 0, 349, 0, 294, 0, 220, 0, 294, 0, 262, 0, 247, 0, 220, 0, 175, 0, 165, 0, 147, 0, 131, 0, 98, 0,
    294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 294, 0, 440, 0, 587, 0, 440, 0, 392, 0, 523, 0, 659, 0, 523, 0, 392, 0, 523, 0, 659, 0, 523, 0, 698, 0, 587, 0, 440, 0, 587, 0, 698, 0, 587, 0, 440, 0, 587, 0, 523, 0, 440, 0, 330, 0, 440, 0, 523, 0, 440, 0, 330, 0, 440, 0, 349, 0, 294, 0, 220, 0, 294, 0, 349, 0, 294, 0, 220, 0, 294, 0, 262, 0, 247, 0, 220, 0, 175, 0, 165, 0, 147, 0, 131, 0, 98, 0,
  },
};

const Pen LED_COLOUR[3] = {
  Pen(255, 0, 0),
  Pen(0, 255, 0),
  Pen(0, 0, 255)
};

uint16_t beat = 0;

uint32_t been_pressed;

void init() {
  set_screen_mode(ScreenMode::lores);

  gpio_init(VBUS_DETECT_PIN);
  gpio_set_dir(VBUS_DETECT_PIN, GPIO_IN);
  gpio_init(CHARGE_STATUS_PIN);
  gpio_set_dir(CHARGE_STATUS_PIN, GPIO_IN);

  channels[0].waveforms   = Waveform::SQUARE;
  channels[0].attack_ms   = 16;
  channels[0].decay_ms    = 168;
  channels[0].sustain     = 0xafff;
  channels[0].release_ms  = 168;

  channels[1].waveforms   = Waveform::SQUARE;
  channels[1].attack_ms   = 38;
  channels[1].decay_ms    = 300;
  channels[1].sustain     = 0;
  channels[1].release_ms  = 0;
}

void render(uint32_t time) {
  bool button_a = buttons & Button::A;
  bool button_b = buttons & Button::B;
  bool button_x = buttons & Button::X;
  bool button_y = buttons & Button::Y;
  bool dpad_l = buttons & Button::DPAD_LEFT;
  bool dpad_r = buttons & Button::DPAD_RIGHT;
  bool dpad_u = buttons & Button::DPAD_UP;
  bool dpad_d = buttons & Button::DPAD_DOWN;

  for(int b = 0; b < screen.bounds.w; b++){
    for(int v = 0; v < screen.bounds.h; v++){
      screen.pen = hsv_to_rgba(float(b) / (float)(screen.bounds.w), 1.0f, float(v) / (float)(screen.bounds.h));
      screen.pixel(Point(b, v));
    }
  }

  screen.pen = dpad_r ? Pen(255, 0, 0) : Pen(128, 128, 128);
  screen.text("R", minimal_font, Point(25, 15), false, center_center);

  screen.pen = dpad_d ? Pen(255, 0, 0) : Pen(128, 128, 128);
  screen.text("D", minimal_font, Point(15, 25), false, center_center);

  screen.pen = dpad_u ? Pen(255, 0, 0) : Pen(128, 128, 128);
  screen.text("U", minimal_font, Point(15, 5), false, center_center);

  screen.pen = dpad_l ? Pen(255, 0, 0) : Pen(128, 128, 128);
  screen.text("L", minimal_font, Point(5, 15), false, center_center);

  screen.pen = button_a ? Pen(255, 0, 0) : Pen(128, 128, 128);
  screen.text("A", minimal_font, Point(screen.bounds.w - 5, 15), false, center_center);

  screen.pen = button_b ? Pen(255, 0, 0) : Pen(128, 128, 128);
  screen.text("B", minimal_font, Point(screen.bounds.w - 15, 25), false, center_center);

  screen.pen = button_x ? Pen(255, 0, 0) : Pen(128, 128, 128);
  screen.text("X", minimal_font, Point(screen.bounds.w - 15, 5), false, center_center);

  screen.pen = button_y ? Pen(255, 0, 0) : Pen(128, 128, 128);
  screen.text("Y", minimal_font, Point(screen.bounds.w - 25, 15), false, center_center);

  //LED = hsv_to_rgba(time / 50.0f, 1.0f, 1.0f);
  LED = LED_COLOUR[(time / 1000) % 3];

  Point location(5, screen.bounds.h - (8 * STATUS_V_SPACING));
  std::string label = "";

  screen.pen = Pen(255, 255, 255);
  uint32_t bit = 256;
  while(bit > 0) {
    bit >>= 1;
    switch(bit) {
      case Button::A:
        label = "A    ";
        break;
      case Button::B:
        label = "B    ";
        break;
      case Button::X:
        label = "X    ";
        break;
      case Button::Y:
        label = "Y    ";
        break;
      case Button::DPAD_UP:
        label = "UP   ";
        break;
      case Button::DPAD_DOWN:
        label = "DOWN ";
        break;
      case Button::DPAD_LEFT:
        label = "LEFT ";
        break;
      case Button::DPAD_RIGHT:
        label = "RIGHT";
        break;
    }

    if (been_pressed & bit) {
      label += " OK";
    }

    screen.text(label, minimal_font, location, false);
    location.y += STATUS_V_SPACING;
  }

  bool charge_status = gpio_get(CHARGE_STATUS_PIN);
  bool vbus_connected = gpio_get(VBUS_DETECT_PIN);

  location = Point(screen.bounds.w / 2, screen.bounds.h - (8 * STATUS_V_SPACING));

  label = "CHG:  ";
  label += charge_status ? "Yes" : "No";
  screen.text(label, minimal_font, location);
  location.y += STATUS_V_SPACING;

  label = "VBUS: ";
  label += vbus_connected ? "Yes" : "No";
  screen.text(label, minimal_font, location);
}

void update(uint32_t time) {
  static uint16_t tick = 0;
  static uint16_t prev_beat = 1;

  been_pressed |= buttons.pressed;

  beat = (tick / 8) % 384; // 125ms per beat
  tick++;

  if (beat == prev_beat) return;
  prev_beat = beat;

  for(uint8_t i = 0; i < 2; i++) {
    if(notes[i][beat] > 0) {
      channels[i].frequency = notes[i][beat];
      channels[i].trigger_attack();
    } else if (notes[i][beat] == -1) {
      channels[i].trigger_release();
    }
  }
}

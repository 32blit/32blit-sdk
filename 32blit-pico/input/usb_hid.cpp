// GPIO dpad + ABXY
#include "input.hpp"

#include "hardware/gpio.h"

#include "pico/binary_info.h"

#include "class/hid/hid.h"

#include "engine/api_private.hpp"
#include "engine/input.hpp"

// from USB code
extern uint32_t hid_gamepad_id;
extern bool hid_keyboard_detected;
extern uint8_t hid_joystick[2];
extern uint8_t hid_hat;
extern uint32_t hid_buttons;
extern uint8_t hid_keys[6];

struct GamepadMapping {
  uint32_t id; // vid:pid
  uint8_t a, b, x, y;
  uint8_t up, down, left, right; // if no hat
  uint8_t menu, home, joystick;
};

#define NO 0xFF

static const GamepadMapping gamepad_mappings[]{
  {0x057E2009,  3,  2,  1,  0, 17, 16, 19, 18,  8, 12, 11}, // Switch Pro Controller
  {0x15320705,  0,  1,  3,  4, NO, NO, NO, NO, 16, 15, 13}, // Razer Raiju Mobile
  {0x20D6A711,  2,  1,  3,  0, NO, NO, NO, NO,  8, 12, 10}, // PowerA wired Switch pro controller
  {0x2DC89018,  0,  1,  3,  4, NO, NO, NO, NO, 10, 11, NO}, // 8BitDo Zero 2
  {0x00000000,  0,  1,  2,  3, NO, NO, NO, NO,  4,  5,  6}  // probably wrong fallback
};

#undef NO

// hat -> dpad
const uint32_t dpad_map[]{
  blit::Button::DPAD_UP,
  blit::Button::DPAD_UP | blit::Button::DPAD_RIGHT,
  blit::Button::DPAD_RIGHT,
  blit::Button::DPAD_DOWN | blit::Button::DPAD_RIGHT,
  blit::Button::DPAD_DOWN,
  blit::Button::DPAD_DOWN | blit::Button::DPAD_LEFT,
  blit::Button::DPAD_LEFT,
  blit::Button::DPAD_UP | blit::Button::DPAD_LEFT,
  0
};

void init_input() {
}

void update_input() {
  using namespace blit;

  // keyboard
  if(hid_keyboard_detected) {
    uint32_t new_buttons = 0;

    for(int i = 0; i < 6; i++) {
      switch(hid_keys[i]) {
        case HID_KEY_ARROW_UP:
        case HID_KEY_W:
          new_buttons |= uint32_t(Button::DPAD_UP);
          break;
        case HID_KEY_ARROW_DOWN:
        case HID_KEY_S:
          new_buttons |= uint32_t(Button::DPAD_DOWN);
          break;
        case HID_KEY_ARROW_LEFT:
        case HID_KEY_A:
          new_buttons |= uint32_t(Button::DPAD_LEFT);
          break;
        case HID_KEY_ARROW_RIGHT:
        case HID_KEY_D:
          new_buttons |= uint32_t(Button::DPAD_RIGHT);
          break;

        case HID_KEY_Z:
        case HID_KEY_U:
          new_buttons |= uint32_t(Button::A);
          break;
        case HID_KEY_X:
        case HID_KEY_I:
          new_buttons |= uint32_t(Button::B);
          break;
        case HID_KEY_C:
        case HID_KEY_O:
          new_buttons |= uint32_t(Button::X);
          break;
        case HID_KEY_V:
        case HID_KEY_P:
          new_buttons |= uint32_t(Button::Y);
          break;

        case HID_KEY_1:
          new_buttons |= uint32_t(Button::HOME);
          break;

        case HID_KEY_2:
        case HID_KEY_ESCAPE:
          new_buttons |= uint32_t(Button::MENU);
          break;

        case HID_KEY_3:
          new_buttons |= uint32_t(Button::JOYSTICK);
          break;
      }
    }

    api_data.buttons = new_buttons;
  }

  if(!hid_gamepad_id)
    return;

  // gamepad
  auto mapping = gamepad_mappings;
  while(mapping->id && mapping->id != hid_gamepad_id)
    mapping++;

  api_data.buttons = dpad_map[hid_hat > 8 ? 8 : hid_hat]
                   | (hid_buttons & (1 << mapping->left)     ? uint32_t(Button::DPAD_LEFT) : 0)
                   | (hid_buttons & (1 << mapping->right)    ? uint32_t(Button::DPAD_RIGHT) : 0)
                   | (hid_buttons & (1 << mapping->up)       ? uint32_t(Button::DPAD_UP) : 0)
                   | (hid_buttons & (1 << mapping->down)     ? uint32_t(Button::DPAD_DOWN) : 0)
                   | (hid_buttons & (1 << mapping->a)        ? uint32_t(Button::A) : 0)
                   | (hid_buttons & (1 << mapping->b)        ? uint32_t(Button::B) : 0)
                   | (hid_buttons & (1 << mapping->x)        ? uint32_t(Button::X) : 0)
                   | (hid_buttons & (1 << mapping->y)        ? uint32_t(Button::Y) : 0)
                   | (hid_buttons & (1 << mapping->menu)     ? uint32_t(Button::MENU) : 0)
                   | (hid_buttons & (1 << mapping->home)     ? uint32_t(Button::HOME) : 0)
                   | (hid_buttons & (1 << mapping->joystick) ? uint32_t(Button::JOYSTICK) : 0);

  api_data.joystick.x = (float(hid_joystick[0]) - 0x80) / 0x80;
  api_data.joystick.y = (float(hid_joystick[1]) - 0x80) / 0x80;
}

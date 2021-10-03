// GPIO dpad + ABXY
#include "input.hpp"

#include "hardware/gpio.h"

#include "pico/binary_info.h"

#include "engine/api_private.hpp"
#include "engine/input.hpp"

// from USB code
extern uint32_t hid_gamepad_id;
extern uint8_t hid_joystick[2];
extern uint8_t hid_hat;
extern uint32_t hid_buttons;
struct GamepadMapping {
  uint32_t id; // vid:pid
  uint8_t a, b, x, y;
  uint8_t menu, home, joystick;
};

static const GamepadMapping gamepad_mappings[]{
  {0x15320705,  0,  1,  3,  4,  16, 15, 13}, // Razer Raiju Mobile
  {0x20D6A711,  2,  1,  3,  0,  8,  12, 10}, // PowerA wired Switch pro controller
  {0x00000000,  0,  1,  2,  3,  4,   5,  6}  // probably wrong fallback
};

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

  if(!hid_gamepad_id)
    return;

  auto mapping = gamepad_mappings;
  while(mapping->id && mapping->id != hid_gamepad_id)
    mapping++;

  api.buttons = dpad_map[hid_hat > 8 ? 8 : hid_hat]
              | (hid_buttons & (1 << mapping->a)        ? uint32_t(Button::A) : 0)
              | (hid_buttons & (1 << mapping->b)        ? uint32_t(Button::B) : 0)
              | (hid_buttons & (1 << mapping->x)        ? uint32_t(Button::X) : 0)
              | (hid_buttons & (1 << mapping->y)        ? uint32_t(Button::Y) : 0)
              | (hid_buttons & (1 << mapping->menu)     ? uint32_t(Button::MENU) : 0)
              | (hid_buttons & (1 << mapping->home)     ? uint32_t(Button::HOME) : 0)
              | (hid_buttons & (1 << mapping->joystick) ? uint32_t(Button::JOYSTICK) : 0);

  api.joystick.x = (float(hid_joystick[0]) - 0x80) / 0x80;
  api.joystick.y = (float(hid_joystick[1]) - 0x80) / 0x80;
}

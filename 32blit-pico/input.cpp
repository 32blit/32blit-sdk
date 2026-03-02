#include <array>

#include "input.hpp"

#include "engine/api_private.hpp"

extern const InputDriver gpio_input_driver;
extern const InputDriver tca9555_driver;
extern const InputDriver usb_hid_driver;

static const InputDriver *input_drivers[] {
#ifdef BLIT_INPUT_GPIO
  &gpio_input_driver,
#endif
#ifdef BLIT_INPUT_TCA9555
  &tca9555_driver,
#endif
#ifdef BLIT_INPUT_USB_HID
  &usb_hid_driver,
#endif
};
static constexpr unsigned num_input_drivers = sizeof(input_drivers) / sizeof(input_drivers[0]);

void init_input() {
  for(unsigned i = 0; i < num_input_drivers; i++)
    input_drivers[i]->init();
}

void update_input() {
  uint32_t new_buttons = 0;
  blit::Vec2 new_joystick;

  for(unsigned i = 0; i < num_input_drivers; i++)
    input_drivers[i]->update(new_buttons, new_joystick);

  blit::api_data.buttons = new_buttons;
  blit::api_data.joystick = new_joystick;
}

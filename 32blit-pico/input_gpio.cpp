// generic GPIO buttons
#include "input.hpp"
#include "hardware/gpio.h"

#include "pico/binary_info.h"

#include "config.h"

#include "engine/api_private.hpp"
#include "engine/input.hpp"

// order matches blit::Button
static constexpr int button_io[] {
  BUTTON_LEFT_PIN,
  BUTTON_RIGHT_PIN,
  BUTTON_UP_PIN,
  BUTTON_DOWN_PIN,

  BUTTON_A_PIN,
  BUTTON_B_PIN,
  BUTTON_X_PIN,
  BUTTON_Y_PIN,

  BUTTON_HOME_PIN,
  BUTTON_MENU_PIN,
  BUTTON_JOYSTICK_PIN,
};

static void init_button(int pin) {
  gpio_set_function(pin, GPIO_FUNC_SIO);
  gpio_set_dir(pin, GPIO_IN);
  gpio_pull_up(pin);
}

void init_input() {
  for(size_t i = 0; i < std::size(button_io); i++)
    init_button(button_io[i]);

  // declare pins
  #define BUTTON_BI_DECL(pin, btn) bi_decl(bi_1pin_with_name(pin, #btn" Button"));
  BUTTON_LEFT_BI_DECL
  BUTTON_RIGHT_BI_DECL
  BUTTON_UP_BI_DECL
  BUTTON_DOWN_BI_DECL
  BUTTON_A_BI_DECL
  BUTTON_B_BI_DECL
  BUTTON_X_BI_DECL
  BUTTON_Y_BI_DECL
  BUTTON_HOME_BI_DECL
  BUTTON_MENU_BI_DECL
  BUTTON_JOYSTICK_BI_DECL
  #undef BUTTON_BI_DECL
}

void update_input() {
  auto io = gpio_get_all();

  uint32_t new_buttons = 0;

  for(std::size_t i = 0; i < std::size(button_io); i++) {
    // pin not defined, skip
    if(button_io[i] == -1)
      continue;
    
    bool pin_state = !!(io & (1 << button_io[i]));

    if(pin_state == false) // active low
      new_buttons |= 1 << i;
  }

  blit::api.buttons = new_buttons;
}

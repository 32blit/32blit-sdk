#include "input.hpp"

#include "hardware/gpio.h"

#include "pico/binary_info.h"

#include "engine/api_private.hpp"
#include "engine/input.hpp"

enum class ButtonIO {
  UP = 22,
  DOWN = 6,

  A = 7,
  B = 8,
  C = 9,
  USER = 23, // USER_SW
};

static void init_button(ButtonIO b) {
  int gpio = static_cast<int>(b);
  gpio_set_function(gpio, GPIO_FUNC_SIO);
  gpio_set_dir(gpio, GPIO_IN);
  gpio_pull_down(gpio);
}

inline bool get_button(ButtonIO b) {
  auto gpio = static_cast<int>(b);

  if(b == ButtonIO::USER) // USER_SW goes the other way
    return !gpio_get(gpio);

  return gpio_get(gpio);
}

void init_input() {
  init_button(ButtonIO::UP);
  init_button(ButtonIO::DOWN);
  init_button(ButtonIO::A);
  init_button(ButtonIO::B);
  init_button(ButtonIO::C);
  init_button(ButtonIO::USER);

  #define BUTTON_DECL(btn) bi_decl(bi_1pin_with_name(static_cast<int>(ButtonIO::btn), #btn" Button"));
  BUTTON_DECL(UP)
  BUTTON_DECL(DOWN)
  BUTTON_DECL(A)
  BUTTON_DECL(B)
  BUTTON_DECL(C)
  BUTTON_DECL(USER)
  #undef BUTTON_DECL
}

void update_input() {
  using namespace blit;

  api.buttons = (get_button(ButtonIO::UP)    ? uint32_t(Button::DPAD_UP) : 0)
              | (get_button(ButtonIO::DOWN)  ? uint32_t(Button::DPAD_DOWN) : 0)
              | (get_button(ButtonIO::A)     ? uint32_t(Button::A) : 0)
              | (get_button(ButtonIO::B)     ? uint32_t(Button::B) : 0)
              | (get_button(ButtonIO::C)     ? uint32_t(Button::X) : 0)
              | (get_button(ButtonIO::USER)  ? uint32_t(Button::Y) : 0);
}

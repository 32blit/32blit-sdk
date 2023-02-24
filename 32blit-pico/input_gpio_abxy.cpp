// GPIO ABXY, common pins on some packs + explorer base
#include "input.hpp"

#include "hardware/gpio.h"

#include "pico/binary_info.h"

#include "engine/api_private.hpp"
#include "engine/input.hpp"

enum class ButtonIO {
  A = 12,
  B = 13,
  X = 14,
  Y = 15,
};

static void init_button(ButtonIO b) {
  int gpio = static_cast<int>(b);
  gpio_set_function(gpio, GPIO_FUNC_SIO);
  gpio_set_dir(gpio, GPIO_IN);
  gpio_pull_up(gpio);
}

inline bool get_button(ButtonIO b) {
  return !gpio_get(static_cast<int>(b));
}

void init_input() {
  init_button(ButtonIO::A);
  init_button(ButtonIO::B);
  init_button(ButtonIO::X);
  init_button(ButtonIO::Y);

  #define BUTTON_DECL(btn) bi_decl(bi_1pin_with_name(static_cast<int>(ButtonIO::btn), #btn" Button"));
  BUTTON_DECL(A)
  BUTTON_DECL(B)
  BUTTON_DECL(X)
  BUTTON_DECL(Y)
  #undef BUTTON_DECL
}

void update_input() {
  using namespace blit;

  api.buttons = (get_button(ButtonIO::A) ? uint32_t(Button::A) : 0)
              | (get_button(ButtonIO::B) ? uint32_t(Button::B) : 0)
              | (get_button(ButtonIO::X) ? uint32_t(Button::X) : 0)
              | (get_button(ButtonIO::Y) ? uint32_t(Button::Y) : 0);
}

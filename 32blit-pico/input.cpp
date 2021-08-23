#include "input.hpp"

#include "hardware/gpio.h"

#include "pico/binary_info.h"

#include "engine/api_private.hpp"
#include "engine/input.hpp"

#ifdef INPUT_GPIO

enum class ButtonIO {
#ifdef PIMORONI_PICOSYSTEM
  UP = 23,
  DOWN = 20,
  LEFT = 22,
  RIGHT = 21,

  A = 18,
  B = 19,
  X = 17,
  Y = 16,
#else
  UP = 2,
  DOWN = 3,
  LEFT = 4,
  RIGHT = 5,

  A = 12,
  B = 13,
  X = 14,
  Y = 15,
#endif
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
#endif

void init_input() {
#ifdef INPUT_GPIO
  init_button(ButtonIO::UP);
  init_button(ButtonIO::DOWN);
  init_button(ButtonIO::LEFT);
  init_button(ButtonIO::RIGHT);
  init_button(ButtonIO::A);
  init_button(ButtonIO::B);
  init_button(ButtonIO::X);
  init_button(ButtonIO::Y);

  #define BUTTON_DECL(btn) bi_decl(bi_1pin_with_name(static_cast<int>(ButtonIO::btn), #btn" Button"));
  BUTTON_DECL(UP)
  BUTTON_DECL(DOWN)
  BUTTON_DECL(LEFT)
  BUTTON_DECL(RIGHT)
  BUTTON_DECL(A)
  BUTTON_DECL(B)
  BUTTON_DECL(X)
  BUTTON_DECL(Y)
  #undef BUTTON_DECL
#endif
}

void update_input() {
  using namespace blit;

#ifdef INPUT_GPIO
  api.buttons = (get_button(ButtonIO::LEFT) ? Button::DPAD_LEFT : 0)
              | (get_button(ButtonIO::RIGHT) ? Button::DPAD_RIGHT : 0)
              | (get_button(ButtonIO::UP) ? Button::DPAD_UP : 0)
              | (get_button(ButtonIO::DOWN) ? Button::DPAD_DOWN : 0)
              | (get_button(ButtonIO::A) ? Button::A : 0)
              | (get_button(ButtonIO::B) ? Button::B : 0)
              | (get_button(ButtonIO::X) ? Button::X : 0)
              | (get_button(ButtonIO::Y) ? Button::Y : 0);
#endif
}

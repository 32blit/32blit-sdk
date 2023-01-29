// GPIO dpad + ABXY
#include "input.hpp"

#include "hardware/gpio.h"

#include "pico/binary_info.h"

#include "engine/api_private.hpp"
#include "engine/input.hpp"

enum class ButtonIO {
  UP = PICOSYSTEM_SW_UP_PIN,
  DOWN = PICOSYSTEM_SW_DOWN_PIN,
  LEFT = PICOSYSTEM_SW_LEFT_PIN,
  RIGHT = PICOSYSTEM_SW_RIGHT_PIN,

  A = PICOSYSTEM_SW_A_PIN,
  B = PICOSYSTEM_SW_B_PIN,
  X = PICOSYSTEM_SW_X_PIN,
  Y = PICOSYSTEM_SW_Y_PIN,
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
}

void update_input() {
  using namespace blit;

  api.buttons = (get_button(ButtonIO::LEFT)  ? uint32_t(Button::DPAD_LEFT) : 0)
              | (get_button(ButtonIO::RIGHT) ? uint32_t(Button::DPAD_RIGHT) : 0)
              | (get_button(ButtonIO::UP)    ? uint32_t(Button::DPAD_UP) : 0)
              | (get_button(ButtonIO::DOWN)  ? uint32_t(Button::DPAD_DOWN) : 0)
              | (get_button(ButtonIO::A)     ? uint32_t(Button::A) : 0)
              | (get_button(ButtonIO::B)     ? uint32_t(Button::B) : 0)
              | (get_button(ButtonIO::X)     ? uint32_t(Button::X) : 0)
              | (get_button(ButtonIO::Y)     ? uint32_t(Button::Y) : 0);
}

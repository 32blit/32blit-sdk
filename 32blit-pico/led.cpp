#include "led.hpp"

#include <cmath>

#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "engine/api_private.hpp"

#ifdef PIMORONI_PICOSYSTEM
static const int led_pins[]{PICOSYSTEM_LED_R_PIN, PICOSYSTEM_LED_G_PIN, PICOSYSTEM_LED_B_PIN};
#define HAVE_LED
#endif

void init_led() {
#ifdef HAVE_LED
  for(auto &pin : led_pins) {
    pwm_config cfg = pwm_get_default_config();
    pwm_set_wrap(pwm_gpio_to_slice_num(pin), 65535);
    pwm_init(pwm_gpio_to_slice_num(pin), &cfg, true);
    gpio_set_function(pin, GPIO_FUNC_PWM);
  }
#endif
}

void update_led() {
  using namespace blit;

#ifdef HAVE_LED
  const float gamma = 2.8;
  uint16_t value = (uint16_t)(std::pow((float)(api.LED.r) / 255.0f, gamma) * 65535.0f + 0.5f);
  pwm_set_gpio_level(led_pins[0], value);
  value = (uint16_t)(std::pow((float)(api.LED.g) / 255.0f, gamma) * 65535.0f + 0.5f);
  pwm_set_gpio_level(led_pins[1], value);
  value = (uint16_t)(std::pow((float)(api.LED.b) / 255.0f, gamma) * 65535.0f + 0.5f);
  pwm_set_gpio_level(led_pins[2], value);
#endif
}

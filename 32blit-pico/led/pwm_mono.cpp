#include "led.hpp"

#include <cmath>

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"

#include "config.h"

#include "engine/api_private.hpp"

static const int led_pins[]{LED_MONO_PINS};

template<typename... Args>
constexpr auto make_mask(Args... args) {
  return ((1ull << args) | ...);
}

void init_led() {
  pwm_config cfg = pwm_get_default_config();
#ifdef LED_INVERTED
  pwm_config_set_output_polarity(&cfg, true, true);
#endif

  for(auto &pin : led_pins) {
    pwm_set_wrap(pwm_gpio_to_slice_num(pin), 65535);
    pwm_init(pwm_gpio_to_slice_num(pin), &cfg, true);
    gpio_set_function(pin, GPIO_FUNC_PWM);
  }

  bi_decl(bi_pin_mask_with_name(make_mask(LED_MONO_PINS), "LED"));
}

void update_led() {
  using namespace blit;

  const float gamma = 2.8;

  float avg = float(api_data.LED.r + api_data.LED.g + api_data.LED.b) / 3.0f;
  uint16_t value = (uint16_t)(std::pow(avg / 255.0f, gamma) * 65535.0f + 0.5f);

  for(auto &pin : led_pins)
    pwm_set_gpio_level(pin, value);
}

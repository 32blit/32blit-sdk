#include "led.hpp"

#include <cmath>

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"

#include "config.h"

#include "engine/api_private.hpp"

#if defined(LED_R_PIN) && defined(LED_G_PIN) && defined(LED_B_PIN)
static const int led_pins[]{LED_R_PIN, LED_G_PIN, LED_B_PIN};
#define HAVE_LED
#elif defined(LED_WS2812_PIN)
#include "hardware/pio.h"
#include "ws2812.pio.h"

static uint32_t last_color = 0;
static int pio_sm = -1;

static void put_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  if(pio_sm < 0) return;
  uint32_t color =
          (((uint32_t)((r * a) >> 8) << 8)
          | ((uint32_t)((g * a) >> 8) << 16)
          | (uint32_t)((b * a) >> 8)) << 8u;
  if(color != last_color) {
    pio_sm_put_blocking(pio0, 0, color);
    last_color = color;
  }
}
#endif

void init_led() {
#ifdef HAVE_LED
  pwm_config cfg = pwm_get_default_config();
#ifdef LED_INVERTED
  pwm_config_set_output_polarity(&cfg, true, true);
#endif

  for(auto &pin : led_pins) {
    pwm_set_wrap(pwm_gpio_to_slice_num(pin), 65535);
    pwm_init(pwm_gpio_to_slice_num(pin), &cfg, true);
    gpio_set_function(pin, GPIO_FUNC_PWM);
  }

  bi_decl(bi_1pin_with_name(led_pins[0], "Red LED"));
  bi_decl(bi_1pin_with_name(led_pins[1], "Green LED"));
  bi_decl(bi_1pin_with_name(led_pins[2], "Blue LED"));
#elif defined(LED_WS2812_PIN)
  PIO pio = pio0;
  uint pio_offset = pio_add_program(pio, &ws2812_program);
  pio_sm = pio_claim_unused_sm(pio, false);
  if(pio_sm > -1) {
    ws2812_program_init(pio, pio_sm, pio_offset, LED_WS2812_PIN, 800000, true);
    pio_sm_put_blocking(pio, 0, 0);
  } else {
    printf("LED_WS2812: could not find a free pio sm\r\n");
  }

  bi_decl(bi_1pin_with_name(LED_WS2812_PIN, "NeoPixel RGB LED (WS2812)"));
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
#elif defined(LED_WS2812_PIN)
  put_pixel(api.LED.r, api.LED.g, api.LED.b, api.LED.a);
#endif
}

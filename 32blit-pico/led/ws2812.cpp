#include "led.hpp"

#include <cmath>

#include "hardware/pio.h"
#include "pico/binary_info.h"

#include "config.h"
#include "ws2812.pio.h"

#include "engine/api_private.hpp"

#define led_pio __CONCAT(pio, LED_WS2812_PIO)

static blit::Pen last_led;
static int pio_sm = -1;

static void put_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  uint32_t color =
          (((uint32_t)((r * a) >> 8) << 8)
          | ((uint32_t)((g * a) >> 8) << 16)
          | (uint32_t)((b * a) >> 8)) << 8u;

  pio_sm_put_blocking(led_pio, 0, color);
}

void init_led() {
#if LED_WS2812_PIN >= 32
  pio_set_gpio_base(led_pio, 16);
#endif

  uint pio_offset = pio_add_program(led_pio, &ws2812_program);
  pio_sm = pio_claim_unused_sm(led_pio, false);
  if(pio_sm > -1) {
    ws2812_program_init(led_pio, pio_sm, pio_offset, LED_WS2812_PIN, 800000, true);
    pio_sm_put_blocking(led_pio, 0, 0);
  } else {
    printf("LED_WS2812: could not find a free pio sm\r\n");
  }

  bi_decl(bi_1pin_with_name(LED_WS2812_PIN, "NeoPixel RGB LED (WS2812)"));
}

void update_led() {
  using namespace blit;

  if(pio_sm < 0) return;

  if(!(api_data.LED == last_led)) {
    put_pixel(api_data.LED.r, api_data.LED.g, api_data.LED.b, api_data.LED.a);
    last_led = api_data.LED;
  }
}

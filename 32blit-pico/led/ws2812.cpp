#include "led.hpp"

#include <cmath>

#include "hardware/pio.h"
#include "pico/binary_info.h"

#include "config.h"
#include "ws2812.pio.h"

#include "engine/api_private.hpp"

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

void init_led() {
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
}

void update_led() {
  using namespace blit;

  put_pixel(api_data.LED.r, api_data.LED.g, api_data.LED.b, api_data.LED.a);
}

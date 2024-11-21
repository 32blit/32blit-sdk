#include <cstdio>

#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "input.hpp"

#include "config.h"

#include "engine/api_private.hpp"
#include "engine/input.hpp"

#ifndef TCA9555_I2C
#define TCA9555_I2C i2c_default
#endif

#ifndef TCA9555_SDA_PIN
#define TCA9555_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN
#endif

#ifndef TCA9555_SCL_PIN
#define TCA9555_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN
#endif

#ifndef TCA9555_ADDR
#define TCA9555_ADDR 0x21
#endif

// QwST Pad
#define TCA9555_LEFT_IO   2
#define TCA9555_RIGHT_IO  3
#define TCA9555_UP_IO     1
#define TCA9555_DOWN_IO   4
#define TCA9555_A_IO      14
#define TCA9555_B_IO      12
#define TCA9555_X_IO      15
#define TCA9555_Y_IO      13
#define TCA9555_START_IO  11
#define TCA9555_SELECT_IO 5

void init_input() {
  // TODO: may want common i2c setup in the future?
  gpio_set_function(TCA9555_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(TCA9555_SCL_PIN, GPIO_FUNC_I2C);
  i2c_init(TCA9555_I2C, 400000);

  // setup for reading
  uint8_t port = 0;
  i2c_write_blocking(TCA9555_I2C, TCA9555_ADDR, &port, 1, true);
}

void update_input() {
  uint16_t gpio = 0;

  i2c_read_blocking(TCA9555_I2C, TCA9555_ADDR, (uint8_t *)&gpio, 2, false);

  uint32_t new_buttons = 0;

  if(!(gpio & (1 << TCA9555_LEFT_IO)))
    new_buttons |= blit::Button::DPAD_LEFT;

  if(!(gpio & (1 << TCA9555_RIGHT_IO)))
    new_buttons |= blit::Button::DPAD_RIGHT;

  if(!(gpio & (1 << TCA9555_UP_IO)))
    new_buttons |= blit::Button::DPAD_UP;

  if(!(gpio & (1 << TCA9555_DOWN_IO)))
    new_buttons |= blit::Button::DPAD_DOWN;

  if(!(gpio & (1 << TCA9555_A_IO)))
    new_buttons |= blit::Button::A;

  if(!(gpio & (1 << TCA9555_B_IO)))
    new_buttons |= blit::Button::B;

  if(!(gpio & (1 << TCA9555_X_IO)))
    new_buttons |= blit::Button::X;

  if(!(gpio & (1 << TCA9555_Y_IO)))
    new_buttons |= blit::Button::Y;

  if(!(gpio & (1 << TCA9555_START_IO)))
    new_buttons |= blit::Button::HOME;

  if(!(gpio & (1 << TCA9555_SELECT_IO)))
    new_buttons |= blit::Button::MENU;

  blit::api_data.buttons = new_buttons;
}

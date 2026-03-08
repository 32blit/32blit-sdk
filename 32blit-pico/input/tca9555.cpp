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

static bool tca9555_found = false;

void init_tca9555() {
  // setup for reading
  uint8_t port = 0;
  tca9555_found = i2c_write_timeout_us(TCA9555_I2C, TCA9555_ADDR, &port, 1, false, 1000) == 1;
}

void update_tca9555(uint32_t &new_buttons, blit::Vec2 &new_joystick) {
  if(!tca9555_found)
    return;

  uint16_t gpio = 0xFFFF;

  if(i2c_read_blocking(TCA9555_I2C, TCA9555_ADDR, (uint8_t *)&gpio, 2, false) != 2) {
    // attempt to reset the address if read fails
    uint8_t port = 0;
    i2c_write_timeout_us(TCA9555_I2C, TCA9555_ADDR, &port, 1, false, 1000);
    return; // will try again next time
  }

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
}

extern const InputDriver tca9555_driver {
  init_tca9555, update_tca9555
};

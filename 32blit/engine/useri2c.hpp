#pragma once

#include <stdint.h>

namespace blit {
  extern void (*i2c_receive) (uint8_t address, uint8_t reg, uint8_t &result, uint8_t result_size);
  extern void (*i2c_send)    (uint8_t address, uint8_t reg, uint8_t &data, uint8_t data_size);
}

#ifndef __PIN_CONFIGURATION_H
#define __PIN_CONFIGURATION_H

#include <cstdint>

#include "main.h"
#include "gpio_defs.h"

namespace gpio {

  // initialises all of the pins of the MCU into the correct
  // configuration for 32blit
  void init();

  inline bool read(GPIO_TypeDef *port, uint16_t pins) {
    return port->IDR & pins;
  }

  inline void write(GPIO_TypeDef *port, uint16_t pins, bool value) {
    if(value)
      port->BSRR = pins;
    else
      port->BSRR = pins << 16;
  }
}

#endif

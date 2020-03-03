#ifndef __PIN_CONFIGURATION_H
#define __PIN_CONFIGURATION_H

#include "main.h"
#include "gpio_defs.h"

namespace gpio {

  // initialises all of the pins of the MCU into the correct 
  // configuration for 32blit    
  void init();

}

#endif 
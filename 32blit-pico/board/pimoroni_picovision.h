#ifndef _BOARDS_PIMORONI_PICOVISION_H
#define _BOARDS_PIMORONI_PICOVISION_H

#define PIMORONI_PICOVISION

// --- I2C ---
#ifndef PICO_DEFAULT_I2C
#define PICO_DEFAULT_I2C 1
#endif
#ifndef PICO_DEFAULT_I2C_SDA_PIN
#define PICO_DEFAULT_I2C_SDA_PIN 6
#endif
#ifndef PICO_DEFAULT_I2C_SCL_PIN
#define PICO_DEFAULT_I2C_SCL_PIN 7
#endif

#include "boards/pico_w.h"

#endif

#pragma once

extern "C" {

  #include "stm32h7xx_hal.h"

  void Error_Handler(void);

  #define HIGH(PORT, PIN) HAL_GPIO_WritePin(PORT, PIN, GPIO_PIN_SET);
  #define LOW(PORT, PIN) HAL_GPIO_WritePin(PORT, PIN, GPIO_PIN_RESET);

  #define DELAY(MS)       HAL_Delay(MS)

}

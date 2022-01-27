#pragma once

#include "stm32h7xx_hal.h"

#include "audio/audio.hpp"

extern TIM_HandleTypeDef htim6;
extern DAC_HandleTypeDef hdac1;

extern "C" {
  void TIM6_DAC_IRQHandler(void);
  void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
}


using namespace blit;

namespace sound {

  extern bool enabled;

  void init();

}

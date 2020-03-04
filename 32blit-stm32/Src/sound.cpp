#include "stm32h7xx_hal.h"

#include "32blit.hpp"
#include "engine/api_private.hpp"
#include "gpio.hpp"
#include "sound.hpp"

TIM_HandleTypeDef htim6;
DAC_HandleTypeDef hdac1;

void TIM6_DAC_IRQHandler(void) {  

  if (__HAL_TIM_GET_FLAG(&htim6, TIM_FLAG_UPDATE) != RESET)
  {
    if (__HAL_TIM_GET_IT_SOURCE(&htim6, TIM_IT_UPDATE) != RESET)
    {
      __HAL_TIM_CLEAR_IT(&htim6, TIM_IT_UPDATE);

      static bool was_amp_enabled = true;
      bool enable_amp = is_audio_playing();

      if(enable_amp != was_amp_enabled) {
        HAL_GPIO_WritePin(AMP_SHUTDOWN_GPIO_Port, AMP_SHUTDOWN_Pin, enable_amp ? GPIO_PIN_SET : GPIO_PIN_RESET);
        was_amp_enabled = enable_amp;
      }

      // timer period elapsed, update audio sample
      hdac1.Instance->DHR12R2 = blit::get_audio_frame() >> 4;
    }
  }
}

namespace sound {

  AudioChannel channels[CHANNEL_COUNT];

  void init() {
    blit::api.channels = channels;

    // setup the 22,010Hz audio timer    
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    __TIM6_CLK_ENABLE();

    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 128;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 85;

    if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
    {
      // TODO: fail
    }    


    __HAL_RCC_DAC12_CLK_ENABLE();
    DAC_ChannelConfTypeDef sConfig = {0};

    // setup the dac output
    hdac1.Instance = DAC1;
    if (HAL_DAC_Init(&hdac1) != HAL_OK)
    {
      // TODO: fail
    }
    sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
    //sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
    sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
    sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
    if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
    {
      // TODO: fail
    }


    HAL_TIM_Base_Start_IT(&htim6);
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);

  }

}



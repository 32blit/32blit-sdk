
#include "adc.h" // HAL stuff
#include "adc.hpp"

#define ADC_BUFFER_SIZE 32
__attribute__((section(".dma_data"))) ALIGN_32BYTES(__IO uint16_t adc1data[ADC_BUFFER_SIZE]);
__attribute__((section(".dma_data"))) ALIGN_32BYTES(__IO uint16_t adc3data[ADC_BUFFER_SIZE]);

// callbacks
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc) {
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
  if(hadc->Instance == ADC1) {
    SCB_InvalidateDCache_by_Addr((uint32_t *) &adc1data[0], ADC_BUFFER_SIZE);
  } else if (hadc->Instance == ADC3) {
    SCB_InvalidateDCache_by_Addr((uint32_t *) &adc3data[0], ADC_BUFFER_SIZE);
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  if(hadc->Instance == ADC1) {
    SCB_InvalidateDCache_by_Addr((uint32_t *) &adc1data[ADC_BUFFER_SIZE / 2], ADC_BUFFER_SIZE / 2);
  } else if (hadc->Instance == ADC3) {
    SCB_InvalidateDCache_by_Addr((uint32_t *) &adc3data[ADC_BUFFER_SIZE / 2], ADC_BUFFER_SIZE / 2);
  }
}

namespace adc {

  void init() {
    MX_ADC1_Init();
    MX_ADC3_Init();

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1data, ADC_BUFFER_SIZE);
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *)adc3data, ADC_BUFFER_SIZE);
  }

  uint16_t get_value(Value v) {
    switch(v) {
      case Value::joystick_x:
        return adc1data[0];
      case Value::joystick_y:
        return adc1data[1];

      case Value::hack_left:
        return adc3data[0];
      case Value::hack_right:
        return adc3data[1];
      case Value::battery_charge:
        return adc3data[2];
    }

    return 0;
  }
}

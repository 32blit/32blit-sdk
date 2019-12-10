/**
  ******************************************************************************
  * File Name          : DAC.c
  * Description        : This file provides code for the configuration
  *                      of the DAC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "dac.h"
#include "fatfs.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

extern uint8_t dma_status;

DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch2;

/* DAC1 init function */
void MX_DAC1_Init(void)
{
  DAC_ChannelConfTypeDef sConfig = {0};

  /** DAC Initialization 
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT2 config 
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_DAC_MspInit(DAC_HandleTypeDef* dacHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  HAL_DMA_MuxSyncConfigTypeDef pSyncConfig= {0};
  if(dacHandle->Instance==DAC1)
  {
  /* USER CODE BEGIN DAC1_MspInit 0 */

  /* USER CODE END DAC1_MspInit 0 */
    /* DAC1 clock enable */
    __HAL_RCC_DAC12_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**DAC1 GPIO Configuration    
    PA5     ------> DAC1_OUT2 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* DAC1 DMA Init */
    /* DAC1_CH2 Init */
    hdma_dac1_ch2.Instance = DMA1_Stream0;
    hdma_dac1_ch2.Init.Request = DMA_REQUEST_DAC2;
    hdma_dac1_ch2.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_dac1_ch2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dac1_ch2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dac1_ch2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dac1_ch2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_dac1_ch2.Init.Mode = DMA_CIRCULAR;
    hdma_dac1_ch2.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_dac1_ch2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_dac1_ch2.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_dac1_ch2.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_dac1_ch2.Init.PeriphBurst = DMA_PBURST_SINGLE;

    if (HAL_DMA_Init(&hdma_dac1_ch2) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(dacHandle,DMA_Handle2,hdma_dac1_ch2);

  /* USER CODE BEGIN DAC1_MspInit 1 */

  /* USER CODE END DAC1_MspInit 1 */
  }
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef* dacHandle)
{

  if(dacHandle->Instance==DAC1)
  {
  /* USER CODE BEGIN DAC1_MspDeInit 0 */

  /* USER CODE END DAC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DAC12_CLK_DISABLE();
  
    /**DAC1 GPIO Configuration    
    PA5     ------> DAC1_OUT2 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);

    /* DAC1 DMA DeInit */
    HAL_DMA_DeInit(dacHandle->DMA_Handle2);
  /* USER CODE BEGIN DAC1_MspDeInit 1 */

  /* USER CODE END DAC1_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/**
  ******************************************************************************
  * File Name          : QUADSPI.c
  * Description        : This file provides code for the configuration
  *                      of the QUADSPI instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "quadspi.h"

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

QSPI_HandleTypeDef hqspi;

/* QUADSPI init function */
void MX_QUADSPI_Init(void)
{

  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 2;
  hqspi.Init.FifoThreshold = 1;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = QSPI_FLASH_SIZE;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;
  hqspi.Init.FlashID = QSPI_FLASH_ID_2;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspInit 0 */

  /* USER CODE END QUADSPI_MspInit 0 */
    /* QUADSPI clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();
  

  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* qspiHandle)
{

  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();
  
    /**QUADSPI GPIO Configuration    
    PB2     ------> QUADSPI_CLK
    PE7     ------> QUADSPI_BK2_IO0
    PE8     ------> QUADSPI_BK2_IO1
    PE9     ------> QUADSPI_BK2_IO2
    PE10     ------> QUADSPI_BK2_IO3
    PC11     ------> QUADSPI_BK2_NCS 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_11);

  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  hqspi: QSPI handle
  * @retval None
  */
void QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef     sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    /* Enable write operations ------------------------------------------ */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = WRITE_ENABLE_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    volatile HAL_StatusTypeDef  status = HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);
    if (status != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure automatic polling mode to wait for write enabling ---- */
    sConfig.Match           = 0x02;
    sConfig.Mask            = 0x02;
    sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval        = 0x10;
    sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    sCommand.Instruction    = READ_STATUS_REG1_CMD;
    sCommand.DataMode       = QSPI_DATA_1_LINE;
    status = HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) ;
    if (status != HAL_OK)
    {
        Error_Handler();
    }
}

static HAL_StatusTypeDef QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi, uint32_t Timeout)
{
    QSPI_CommandTypeDef     s_command;
    QSPI_AutoPollingTypeDef s_config;

    /* Configure automatic polling mode to wait for memory ready */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = READ_STATUS_REG1_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    s_config.Match           = 0;
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;
    s_config.Interval        = 0x10;
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;
    s_config.Mask            = 0x01;
    s_config.StatusBytesSize = 1;

    HAL_StatusTypeDef status = HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, Timeout);

    return status;
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static void QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef sCommand = {0};
    uint8_t reg;

    /* Read Volatile Configuration register --------------------------- */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_VOL_CFG_REG_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.NbData            = 1;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_QSPI_Receive(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }

    /* Enable write operations ---------------------------------------- */
    QSPI_WriteEnable(hqspi);

    /* Write Volatile Configuration register (with new dummy cycles) -- */
    sCommand.Instruction = WRITE_VOL_CFG_REG_CMD;
    MODIFY_REG(reg, 0xF0, (DUMMY_CLOCK_CYCLES_READ_QUAD << POSITION_VAL(0xF0)));

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_QSPI_Transmit(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }
}

static uint8_t qspi_reset(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef s_command = {0};

    /* Initialize the reset enable command */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = RESET_ENABLE_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Send the command */
    if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }

    /* Send the reset memory command */
    s_command.Instruction = RESET_MEMORY_CMD;
    if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure automatic polling mode to wait the memory is ready */
    if (QSPI_AutoPollingMemReady(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
    {
        Error_Handler();
    }

    return HAL_OK;
}

void qspi_chip_erase()
{
    QSPI_WriteEnable(&hqspi);

    QSPI_CommandTypeDef s_command = {0};
    s_command.Instruction       = BULK_ERASE_CMD;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }

    QSPI_AutoPollingMemReady(&hqspi, FLASH_CHIP_ERASE_MAX_TIME);
    QSPI_WriteEnable(&hqspi);
}

HAL_StatusTypeDef qspi_sector_erase(uint32_t BlockAddress)
{
    QSPI_WriteEnable(&hqspi);

    QSPI_CommandTypeDef s_command = {0};
    HAL_StatusTypeDef status = QSPI_OK;

    /* Initialize the erase command */
    s_command.Instruction       = SECTOR_ERASE_CMD;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.Address           = BlockAddress;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Send the command */
    if ((status = HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
    {
        Error_Handler();
    }

    QSPI_AutoPollingMemReady(&hqspi, FLASH_SECTOR_ERASE_MAX_TIME);

    return status;
}

HAL_StatusTypeDef qspi_write_buffer(size_t offset, const uint8_t* buffer, size_t length)
{
    QSPI_WriteEnable(&hqspi);

    QSPI_CommandTypeDef s_command = {0};
    HAL_StatusTypeDef status = QSPI_OK;

    s_command.Instruction = QUAD_IN_FAST_PROG_CMD;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DummyCycles = 0;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    s_command.AddressMode = QSPI_ADDRESS_1_LINE;
    s_command.DataMode    = QSPI_DATA_4_LINES;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;

    s_command.Address     = offset;
    s_command.NbData      = length;

    if ((status = HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) == HAL_OK)
    {
    	if ((status = HAL_QSPI_Transmit(&hqspi, (uint8_t *)buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE )) == HAL_OK)
    	{
        status = QSPI_AutoPollingMemReady(&hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);
    	}
    }

    return status;
}

HAL_StatusTypeDef qspi_read_buffer(size_t address, const uint8_t* buffer, size_t length) {

    QSPI_CommandTypeDef s_command = {0};
    HAL_StatusTypeDef status = QSPI_OK;

    s_command.Instruction = QUAD_OUT_FAST_READ_CMD;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DummyCycles = 0;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    s_command.DummyCycles = DUMMY_CLOCK_CYCLES_READ_QUAD;
    s_command.AddressMode = QSPI_ADDRESS_1_LINE;
    s_command.DataMode    = QSPI_DATA_4_LINES;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;

    s_command.Address     = address;
    s_command.NbData      = length;

    if ((status = HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) == HAL_OK)
    {
    	status = HAL_QSPI_Receive(&hqspi, (uint8_t *)buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);
    }

    return status;
}

static HAL_StatusTypeDef qspi_enable_quad(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef s_command = {0};

    uint8_t status[1] = {0x02};
    s_command.Instruction = WRITE_STATUS_REG2;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_1_LINE;
    s_command.DummyCycles = 0;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    s_command.NbData      = sizeof(status);

    if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_QSPI_Transmit(hqspi, status, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    return QSPI_OK;
}

HAL_StatusTypeDef qspi_enable_memorymapped_mode(void)
{
    QSPI_CommandTypeDef      s_command = {0};
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.DataMode          = QSPI_DATA_4_LINES;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DummyCycles       = 6;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    s_command.Instruction       = QUAD_OUT_FAST_READ_DTR_CMD;
    s_command.DdrMode           = QSPI_DDR_MODE_ENABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_HALF_CLK_DELAY;

    /* Configure the memory mapped mode */
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod     = 0;

    if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)
    {
        Error_Handler();
    }

    return QSPI_OK;
}

// for some godforsaken reason if this is optimised in any way when returning from the function
// it jumps off into the normal SPI code!
#pragma GCC push_options
#pragma GCC optimize ("O0")
int qspi_init(void) {
  qspi_reset(&hqspi);
  qspi_enable_quad(&hqspi);
  return(0);
}
#pragma GCC pop_options

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

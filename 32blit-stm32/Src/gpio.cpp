#include "gpio.hpp"

namespace gpio {

  void init_pin(GPIO_TypeDef *port, uint32_t pin, uint32_t mode, uint32_t pull, uint32_t speed = 0, uint32_t alt = 0) {
    GPIO_InitTypeDef gpio = {pin, mode, pull, speed, alt};
    HAL_GPIO_Init(port, &gpio);
  }

  // initialises all of the pins of the MCU into the correct 
  // configuration for 32blit    
  void init()
  {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    // set initial output states where needed
    HAL_GPIO_WritePin(AMP_SHUTDOWN_GPIO_Port, AMP_SHUTDOWN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, LCD_CS_Pin|LCD_RESET_Pin|SD_SPI1_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(USB_SWAP_GPIO_Port, USB_SWAP_Pin, GPIO_PIN_RESET);

    // usb otg
    init_pin(GPIOB, GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF12_OTG2_FS);

    // usb vbus detect
    init_pin(GPIOB, GPIO_PIN_13, GPIO_MODE_INPUT, GPIO_NOPULL);

    // usb "swap" pin?
    init_pin(GPIOD, USB_SWAP_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

#if (INITIALISE_QSPI==1)
// Guard against user code changing QSPI pin state set by firmware
    // qspi
    init_pin(GPIOB, GPIO_PIN_2, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF9_QUADSPI);
    init_pin(GPIOE, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF10_QUADSPI);
    init_pin(GPIOC, GPIO_PIN_11, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF9_QUADSPI);
#endif

    // spi1
    init_pin(GPIOA, SD_SPI1_MOSI_Pin, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI1);
    init_pin(GPIOB, SD_SPI1_MISO_Pin | SD_SPI1_SCK_Pin, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI1);

    // spi4
    init_pin(GPIOE, LCD_SPI4_MOSI_Pin | LCD_SPI4_SCK_Pin, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF5_SPI4);

    // i2c4
    init_pin(GPIOD, GPIO_PIN_12 | GPIO_PIN_13, GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF4_I2C4);

    // dac1
    init_pin(GPIOA, GPIO_PIN_5, GPIO_MODE_ANALOG, GPIO_NOPULL);

    // joystick axes
    init_pin(GPIOC, JOYSTICK_Y_Pin | JOYSTICK_X_Pin, GPIO_MODE_ANALOG, GPIO_NOPULL);
    init_pin(GPIOC, JOYSTICK_BUTTON_Pin, GPIO_MODE_INPUT, GPIO_PULLUP); // button

    // dpad/action buttons
    auto button_pins = DPAD_UP_Pin | DPAD_LEFT_Pin | DPAD_DOWN_Pin | DPAD_RIGHT_Pin | BUTTON_X_Pin | BUTTON_Y_Pin | BUTTON_A_Pin | BUTTON_B_Pin;
    init_pin(GPIOD, button_pins, GPIO_MODE_INPUT, GPIO_PULLUP);

    // system buttons
    init_pin(GPIOD, BUTTON_MENU_Pin, GPIO_MODE_IT_RISING_FALLING, GPIO_PULLUP);
    init_pin(GPIOD, BUTTON_HOME_Pin, GPIO_MODE_INPUT, GPIO_PULLDOWN);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    // user hack headers
    init_pin(GPIOC, USER_LEFT1_Pin | USER_RIGHT1_Pin, GPIO_MODE_ANALOG, GPIO_NOPULL); // analog
    init_pin(GPIOC, USER_LEFT2_Pin | USER_RIGHT2_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL); // digital

    // battery sense
    init_pin(GPIOC, BATTERY_SENSE_Pin, GPIO_MODE_ANALOG, GPIO_NOPULL); // battery sense

    
    // "gpio" pin on extension header
    init_pin(GPIOC, EXTENSION_GPIO_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL); // left digital

    // vibration motor
    init_pin(GPIOB, VIBE_EN_Pin, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH,  GPIO_AF2_TIM4);

    // backlight pwm
    init_pin(GPIOE, LCD_BACKLIGHT_EN_Pin, GPIO_MODE_AF_OD, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH,  GPIO_AF4_TIM15);

    // audio amp enable
    init_pin(GPIOC, AMP_SHUTDOWN_Pin, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP);

    // rgb led pwm
    init_pin(GPIOC, LED_RED_Pin | LED_GREEN_Pin, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF2_TIM3);
    init_pin(GPIOB, LED_BLUE_Pin, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF2_TIM3);

    // lcd
    init_pin(GPIOA, LCD_CS_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL); // chip select
    init_pin(GPIOA, LCD_RESET_Pin, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP); // reset

    // sd card
    init_pin(GPIOA, SD_SPI1_CS_Pin, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP); // chip select
    init_pin(GPIOD, SD_DETECT_Pin, GPIO_MODE_INPUT, GPIO_PULLUP); // card detect

    // ltdc interface
    uint32_t ltdc_a14 = 0, ltdc_b9 = 0, ltdc_b14 = 0, ltdc_c = 0, ltdc_d = 0, ltdc_e = 0;
    ltdc_a14 |= GPIO_PIN_4; // vsync
    ltdc_c   |= GPIO_PIN_6; // hsync
    ltdc_e   |= GPIO_PIN_13; // de
    ltdc_e   |= GPIO_PIN_14; // clk

    // ltdc interface red
    ltdc_a14 |= GPIO_PIN_1; // r2
    ltdc_b9  |= GPIO_PIN_0; // r3
    ltdc_a14 |= GPIO_PIN_11; // r4
    ltdc_a14 |= GPIO_PIN_9; // r5
    ltdc_b9  |= GPIO_PIN_1; // r6
    ltdc_e   |= GPIO_PIN_15; // r7

    // ltdc interface green
    ltdc_a14 |= GPIO_PIN_6; // g2
    ltdc_e   |= GPIO_PIN_11; // g3
    ltdc_b14 |= GPIO_PIN_10; // g4
    ltdc_b14 |= GPIO_PIN_11; // g5
    ltdc_c   |= GPIO_PIN_7; // g6
    ltdc_d   |= GPIO_PIN_3; // g7

    // ltdc interface blue
    ltdc_d   |= GPIO_PIN_6; // b2
    init_pin(GPIOA, GPIO_PIN_8, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF13_LTDC); // b3
    ltdc_e   |= GPIO_PIN_12; // b4
    ltdc_a14 |= GPIO_PIN_3; // b5
    ltdc_b14 |= GPIO_PIN_8; // b6
    ltdc_b14 |= GPIO_PIN_9; // b7

    init_pin(GPIOA, ltdc_a14, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF14_LTDC);
    init_pin(GPIOB, ltdc_b9, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF9_LTDC);
    init_pin(GPIOB, ltdc_b14, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF14_LTDC);
    init_pin(GPIOC, ltdc_c, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF14_LTDC);
    init_pin(GPIOD, ltdc_d, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF14_LTDC);
    init_pin(GPIOE, ltdc_e, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF14_LTDC);

    // beta/final versioning pin
    init_pin(GPIOE, VERSION_Pin, GPIO_MODE_INPUT, GPIO_PULLUP);
  }
}

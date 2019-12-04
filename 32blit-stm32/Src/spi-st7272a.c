#include "spi-st7272a.h"

void st7272a_set_bgr() {
  uint8_t lcd_bgr[2] = {ST7272A_DISPLAY_MODE_REGISTER, ST7272A_DISPLAY_MODE_DEFAULT | 0b00010000}; // Set the SBGR bit to swap DR[7:0] with DB[7:0]
  ST7272A_CS(1);
  HAL_SPI_Transmit(&hspi4, lcd_bgr, 2, HAL_MAX_DELAY);
  ST7272A_CS(0);
}
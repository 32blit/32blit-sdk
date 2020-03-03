#include "spi.h"
#include "gpio_defs.h"
#ifndef __ST7272A_H__

#define ST7272A_DISPLAY_MODE_REGISTER 0x19
#define ST7272A_DISPLAY_MODE_DEFAULT  0xec  // 0b11101100 VA Mode, Top/Bottom & Left/Right scan, Negative polarity VSync/HSync pulses

#define ST7272A_CS(active)            if(active) {LOW(LCD_CS_GPIO_Port, LCD_CS_Pin)} else {HIGH(LCD_CS_GPIO_Port, LCD_CS_Pin)}
#define ST7272A_RESET()               LOW(LCD_RESET_GPIO_Port, LCD_RESET_Pin); DELAY(100); HIGH(LCD_RESET_GPIO_Port, LCD_RESET_Pin); DELAY(100);

#ifdef __cplusplus
extern "C" {
#endif

extern void st7272a_set_bgr();

#ifdef __cplusplus
}
#endif

#endif
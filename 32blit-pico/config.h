#pragma once

#include BLIT_BOARD_CONFIG

// these are the defaults

#ifndef ALLOW_HIRES
#define ALLOW_HIRES 1
#endif

#ifndef DEFAULT_SCREEN_FORMAT
#define DEFAULT_SCREEN_FORMAT PixelFormat::RGB565
#endif

#ifndef DISPLAY_WIDTH
#ifdef DISPLAY_ST7789
#define DISPLAY_WIDTH 240
#else
#define DISPLAY_WIDTH 320
#endif
#endif

#ifndef DISPLAY_HEIGHT
#define DISPLAY_HEIGHT 240
#endif

#ifndef LCD_CS_PIN
#define LCD_CS_PIN PICO_DEFAULT_SPI_CSN_PIN
#endif

#ifndef LCD_DC_PIN
#define LCD_DC_PIN 16
#endif

#ifndef LCD_SCK_PIN
#define LCD_SCK_PIN PICO_DEFAULT_SPI_SCK_PIN
#endif

#ifndef LCD_MOSI_PIN
#define LCD_MOSI_PIN PICO_DEFAULT_SPI_TX_PIN
#endif

#ifndef LCD_BACKLIGHT_PIN
#define LCD_BACKLIGHT_PIN 20
#endif

#ifndef OVERCLOCK_250
#define OVERCLOCK_250 1
#endif

#ifndef USB_VENDOR_ID
#define USB_VENDOR_ID 0xCafe
#endif

#ifndef USB_VENDOR_STR
#define USB_VENDOR_STR "TinyUSB"
#endif

#ifndef USB_PRODUCT_STR
#define USB_PRODUCT_STR "Device"
#endif

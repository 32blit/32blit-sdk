#pragma once

#ifdef BLIT_BOARD_CONFIG
#include BLIT_BOARD_CONFIG
#endif

// these are the defaults

#ifndef ALLOW_HIRES
#define ALLOW_HIRES 1
#endif

#ifndef DOUBLE_BUFFERED_HIRES
#ifdef PICO_RP2350
#define DOUBLE_BUFFERED_HIRES 1
#else
#define DOUBLE_BUFFERED_HIRES 0
#endif
#endif

// default i2s config to values from SDK board
#ifndef AUDIO_I2S_DATA_PIN
#define AUDIO_I2S_DATA_PIN PICO_AUDIO_I2S_DATA_PIN
#endif

#ifndef AUDIO_I2S_CLOCK_PIN_BASE
#define AUDIO_I2S_CLOCK_PIN_BASE PICO_AUDIO_I2S_CLOCK_PIN_BASE
#endif

#ifndef AUDIO_I2S_PIO
#define AUDIO_I2S_PIO 0
#endif

#ifndef BUTTON_LEFT_PIN
#define BUTTON_LEFT_PIN -1
#define BUTTON_LEFT_BI_DECL
#else
#define BUTTON_LEFT_BI_DECL BUTTON_BI_DECL(BUTTON_LEFT_PIN, Left)
#endif

#ifndef BUTTON_LEFT_ACTIVE_HIGH
#define BUTTON_LEFT_ACTIVE_HIGH false
#endif

#ifndef BUTTON_RIGHT_PIN
#define BUTTON_RIGHT_PIN -1
#define BUTTON_RIGHT_BI_DECL
#else
#define BUTTON_RIGHT_BI_DECL BUTTON_BI_DECL(BUTTON_RIGHT_PIN, Right)
#endif

#ifndef BUTTON_RIGHT_ACTIVE_HIGH
#define BUTTON_RIGHT_ACTIVE_HIGH false
#endif

#ifndef BUTTON_UP_PIN
#define BUTTON_UP_PIN -1
#define BUTTON_UP_BI_DECL
#else
#define BUTTON_UP_BI_DECL BUTTON_BI_DECL(BUTTON_UP_PIN, Up)
#endif

#ifndef BUTTON_UP_ACTIVE_HIGH
#define BUTTON_UP_ACTIVE_HIGH false
#endif

#ifndef BUTTON_DOWN_PIN
#define BUTTON_DOWN_PIN -1
#define BUTTON_DOWN_BI_DECL
#else
#define BUTTON_DOWN_BI_DECL BUTTON_BI_DECL(BUTTON_DOWN_PIN, Down)
#endif

#ifndef BUTTON_DOWN_ACTIVE_HIGH
#define BUTTON_DOWN_ACTIVE_HIGH false
#endif

#ifndef BUTTON_A_PIN
#define BUTTON_A_PIN -1
#define BUTTON_A_BI_DECL
#else
#define BUTTON_A_BI_DECL BUTTON_BI_DECL(BUTTON_A_PIN, A)
#endif

#ifndef BUTTON_A_ACTIVE_HIGH
#define BUTTON_A_ACTIVE_HIGH false
#endif

#ifndef BUTTON_B_PIN
#define BUTTON_B_PIN -1
#define BUTTON_B_BI_DECL
#else
#define BUTTON_B_BI_DECL BUTTON_BI_DECL(BUTTON_B_PIN, B)
#endif

#ifndef BUTTON_B_ACTIVE_HIGH
#define BUTTON_B_ACTIVE_HIGH false
#endif

#ifndef BUTTON_X_PIN
#define BUTTON_X_PIN -1
#define BUTTON_X_BI_DECL
#else
#define BUTTON_X_BI_DECL BUTTON_BI_DECL(BUTTON_X_PIN, X)
#endif

#ifndef BUTTON_X_ACTIVE_HIGH
#define BUTTON_X_ACTIVE_HIGH false
#endif

#ifndef BUTTON_Y_PIN
#define BUTTON_Y_PIN -1
#define BUTTON_Y_BI_DECL
#else
#define BUTTON_Y_BI_DECL BUTTON_BI_DECL(BUTTON_Y_PIN, Y)
#endif

#ifndef BUTTON_Y_ACTIVE_HIGH
#define BUTTON_Y_ACTIVE_HIGH false
#endif

#ifndef BUTTON_MENU_PIN
#define BUTTON_MENU_PIN -1
#define BUTTON_MENU_BI_DECL
#else
#define BUTTON_MENU_BI_DECL BUTTON_BI_DECL(BUTTON_MENU_PIN, Menu)
#endif

#ifndef BUTTON_MENU_ACTIVE_HIGH
#define BUTTON_MENU_ACTIVE_HIGH false
#endif

#ifndef BUTTON_HOME_PIN
#define BUTTON_HOME_PIN -1
#define BUTTON_HOME_BI_DECL
#else
#define BUTTON_HOME_BI_DECL BUTTON_BI_DECL(BUTTON_HOME_PIN, Home)
#endif

#ifndef BUTTON_HOME_ACTIVE_HIGH
#define BUTTON_HOME_ACTIVE_HIGH false
#endif

#ifndef BUTTON_JOYSTICK_PIN
#define BUTTON_JOYSTICK_PIN -1
#define BUTTON_JOYSTICK_BI_DECL
#else
#define BUTTON_JOYSTICK_BI_DECL BUTTON_BI_DECL(BUTTON_JOYSTICK_PIN, Joystick)
#endif

#ifndef BUTTON_JOYSTICK_ACTIVE_HIGH
#define BUTTON_JOYSTICK_ACTIVE_HIGH false
#endif

#ifndef DEFAULT_SCREEN_FORMAT
#define DEFAULT_SCREEN_FORMAT PixelFormat::RGB565
#endif

#ifndef DISPLAY_WIDTH
#define DISPLAY_WIDTH 320
#endif

#ifndef DISPLAY_HEIGHT
#define DISPLAY_HEIGHT 240
#endif

#if ALLOW_HIRES && DOUBLE_BUFFERED_HIRES
#define FRAMEBUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT * 2)
#elif ALLOW_HIRES
#define FRAMEBUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT)
#else
// height rounded up to handle the 135px display
#define FRAMEBUFFER_SIZE ((DISPLAY_WIDTH / 2) * ((DISPLAY_HEIGHT + 1) / 2) * 2) // double-buffered
#endif

// default flash storage to last 1/4 of flash
#ifndef FLASH_STORAGE_SIZE
#define FLASH_STORAGE_SIZE PICO_FLASH_SIZE_BYTES / 4
#endif

#ifndef FLASH_STORAGE_OFFSET
#define FLASH_STORAGE_OFFSET PICO_FLASH_SIZE_BYTES - FLASH_STORAGE_SIZE
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

#ifndef LCD_ROTATION
#define LCD_ROTATION 0
#endif

#ifndef LCD_MAX_CLOCK
#define LCD_MAX_CLOCK 62500000
#endif

#ifndef LCD_TRANSPOSE
#define LCD_TRANSPOSE 0
#endif

#ifndef LED_WS2812_COUNT
#define LED_WS2812_COUNT 1
#endif

#ifndef LED_WS2812_PIO
#define LED_WS2812_PIO 0
#endif

#ifndef LED_WS2812_RGBW
#define LED_WS2812_RGBW false
#endif

#ifndef OVERCLOCK_250
#define OVERCLOCK_250 1
#endif

#ifndef SD_SPI_OVERCLOCK
#define SD_SPI_OVERCLOCK 1
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

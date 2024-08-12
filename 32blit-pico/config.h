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

#ifndef LCD_ROTATION
#define LCD_ROTATION 0
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

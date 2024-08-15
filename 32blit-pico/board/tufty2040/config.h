#pragma once

#define BUTTON_UP_PIN   22
#define BUTTON_DOWN_PIN  6
#define BUTTON_A_PIN     7
#define BUTTON_B_PIN     8
#define BUTTON_X_PIN     9 // C
#define BUTTON_Y_PIN    23 // USER_SW

#define BUTTON_UP_ACTIVE_HIGH   true
#define BUTTON_DOWN_ACTIVE_HIGH true
#define BUTTON_A_ACTIVE_HIGH    true
#define BUTTON_B_ACTIVE_HIGH    true
#define BUTTON_X_ACTIVE_HIGH    true

#define DISPLAY_ST7789

#define DBI_8BIT
#define LCD_ROTATION 270
#define LCD_CS_PIN 10
#define LCD_DC_PIN 11
#define LCD_SCK_PIN 12 // WR
#define LCD_RD_PIN 13
#define LCD_MOSI_PIN 14 // DB0
#define LCD_BACKLIGHT_PIN 2
// #define LCD_VSYNC_PIN 11 // shared
#define LCD_MAX_CLOCK 15000000

// there is a white LED
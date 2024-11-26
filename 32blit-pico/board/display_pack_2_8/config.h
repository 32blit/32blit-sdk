#pragma once

#define BUTTON_A_PIN 12
#define BUTTON_B_PIN 13
#define BUTTON_X_PIN 14
#define BUTTON_Y_PIN 15

// these are the defaults on a pico, but may not be on other boards
#define LCD_CS_PIN 17
#define LCD_SCK_PIN 18
#define LCD_MOSI_PIN 19
//#define LCD_VSYNC_PIN 21 // not so useful if rotated, also can't keep up with 50Hz in lores mode

#define LCD_ROTATION 90

#define LED_INVERTED
#define LED_R_PIN 26
#define LED_G_PIN 27
#define LED_B_PIN 28
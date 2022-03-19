#pragma once
// these are the defaults

#ifndef ALLOW_HIRES
#define ALLOW_HIRES 1
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

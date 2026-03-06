

#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include "pico/stdio.h"
#include "pico/sync.h"
#include "hardware/gpio.h"
#include "hardware/powman.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
//#include "hardware/adc.h"
#include "hardware/structs/usb.h"
#include "hardware/structs/xosc.h"
#include "hardware/vreg.h"
#include "hardware/flash.h"
#include "hardware/structs/qmi.h"
#include "hardware/i2c.h"
#include "hardware/resets.h"
#include "hardware/pwm.h"

// For machine_pin_find
//#include "machine_pin.h"

// For Blinky teardown
#include "hardware/pio.h"

#define POWMAN_DOUBLE_RESET_TIMEOUT_MS 1000
#define POWMAN_LONG_PRESS_TIMEOUT_MS 1000

#define POWMAN_WAKE_PWRUP0_CH 0  // WAKE_VBUS_DETECT
#define POWMAN_WAKE_PWRUP1_CH 1  // WAKE_RTC
#define POWMAN_WAKE_PWRUP2_CH 2  // WAKE_USER_SW
#define POWMAN_WAKE_PWRUP3_CH 3  // One of the user buttons

#define POWMAN_WAKE_RESET  0b00000001
#define POWMAN_WAKE_PWRUP0 0b00000010
#define POWMAN_WAKE_PWRUP1 0b00000100
#define POWMAN_WAKE_PWRUP2 0b00001000
#define POWMAN_WAKE_PWRUP3 0b00010000
#define POWMAN_WAKE_CORESI 0b00100000
#define POWMAN_WAKE_ALARM  0b01000000
#define POWMAN_DOUBLETAP   0b10000000

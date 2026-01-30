/*
 * Copyright (c) 2024 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// -----------------------------------------------------
// NOTE: THIS HEADER IS ALSO INCLUDED BY ASSEMBLER SO
//       SHOULD ONLY CONSIST OF PREPROCESSOR DIRECTIVES
// -----------------------------------------------------

// pico_cmake_set PICO_PLATFORM=rp2350
// pico_cmake_set PICO_CYW43_SUPPORTED = 1

#ifndef _BOARDS_PICO2_W_H
#define _BOARDS_PICO2_W_H


//#define PICO_PANIC_FUNCTION mp_pico_panic

// Board config
// RTC = PCF85063e
#define BW_RTC_I2C       i2c0
#define BW_RTC_ADDR      (0x51)
#define BW_RTC_I2C_SDA   (4)
#define BW_RTC_I2C_SCL   (5)

// Rear white LEDs
#define BW_LED_0         (0)
#define BW_LED_1         (1)
#define BW_LED_2         (2)
#define BW_LED_3         (3)

// PSRAM
#define BW_PSRAM_CS      (8)

// User inputs
#define BW_SWITCH_A      (7)
#define BW_SWITCH_B      (9)
#define BW_SWITCH_C      (10)
#define BW_SWITCH_UP     (11)
#define BW_SWITCH_DOWN   (6)

// This is wired to the RESET (Disk / Sleep / Reset / Power On)
// button and used to determine long press status
#define BW_RESET_SW      (14) // No pull, active high?

// I2C power for talking to RTC
#define BW_SW_POWER_EN   (41)

// Interrupt channels for GPIO wakeup
#define BW_VBUS_DETECT   (12) // No pull, active high?
#define BW_RTC_ALARM     (13) // Pull up, active low
#define BW_SWITCH_HOME   (22) // AKA boot
#define BW_SWITCH_INT    (15) // Pull up, active low
#define BW_SWITCH_MASK   ((1 << BW_SWITCH_A) | (1 << BW_SWITCH_B) | (1 << BW_SWITCH_C) | (1 << BW_SWITCH_UP) | (1 << BW_SWITCH_DOWN))

// 250 MHz - Working, but not tested on enough boards to fully verify.
// ./build/micropython/lib/pico-sdk/src/rp2_common/hardware_clocks/scripts/vcocalc.py --cmake 250
/*
#define PLL_SYS_REFDIV   (1)
#define PLL_SYS_VCO_FREQ_HZ (1500000000)
#define PLL_SYS_POSTDIV1 (6)
#define PLL_SYS_POSTDIV2 (1)
#define SYS_CLK_HZ       (250000000)
*/

// 200 MHz - Most of our testing was done at 200MHz.
// ./build/micropython/lib/pico-sdk/src/rp2_common/hardware_clocks/scripts/vcocalc.py --cmake 200
#define PLL_SYS_REFDIV   (1)
#define PLL_SYS_VCO_FREQ_HZ (1200000000)
#define PLL_SYS_POSTDIV1 (6)
#define PLL_SYS_POSTDIV2 (1)
#define SYS_CLK_HZ       (200000000)

// Support 250MHz if user manually overclocks
#define CYW43_PIO_CLOCK_DIV_INT 3

// --- RP2350 VARIANT ---
// not PICO_RP2350A

// --- UART ---
// no PICO_DEFAULT_UART
// no PICO_DEFAULT_UART_TX_PIN
// no PICO_DEFAULT_UART_RX_PIN

// --- LED ---
// no PICO_DEFAULT_LED_PIN - LED is on Wireless chip
// no PICO_DEFAULT_WS2812_PIN

// --- I2C ---
#ifndef PICO_DEFAULT_I2C
#define PICO_DEFAULT_I2C 0
#endif
#ifndef PICO_DEFAULT_I2C_SDA_PIN
#define PICO_DEFAULT_I2C_SDA_PIN BW_RTC_I2C_SDA
#endif
#ifndef PICO_DEFAULT_I2C_SCL_PIN
#define PICO_DEFAULT_I2C_SCL_PIN BW_RTC_I2C_SCL
#endif

// --- SPI ---
// no PICO_DEFAULT_SPI
// no PICO_DEFAULT_SPI_SCK_PIN
// no PICO_DEFAULT_SPI_TX_PIN
// no PICO_DEFAULT_SPI_RX_PIN
// no PICO_DEFAULT_SPI_CSN_PIN

// --- FLASH ---

#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1

#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif

// pico_cmake_set_default PICO_FLASH_SIZE_BYTES = (16 * 1024 * 1024)
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)
#endif

// --- CYW43 ---

#ifndef CYW43_WL_GPIO_COUNT
#define CYW43_WL_GPIO_COUNT 3
#endif

// pico_cmake_set_default PICO_RP2350_A2_SUPPORTED = 1
#ifndef PICO_RP2350_A2_SUPPORTED
#define PICO_RP2350_A2_SUPPORTED 1
#endif

// cyw43 SPI pins can't be changed at runtime
#ifndef CYW43_PIN_WL_DYNAMIC
#define CYW43_PIN_WL_DYNAMIC 0
#endif

// gpio pin to power up the cyw43 chip
#ifndef CYW43_DEFAULT_PIN_WL_REG_ON
#define CYW43_DEFAULT_PIN_WL_REG_ON 23u
#endif

// gpio pin for spi data out to the cyw43 chip
#ifndef CYW43_DEFAULT_PIN_WL_DATA_OUT
#define CYW43_DEFAULT_PIN_WL_DATA_OUT 24u
#endif

// gpio pin for spi data in from the cyw43 chip
#ifndef CYW43_DEFAULT_PIN_WL_DATA_IN
#define CYW43_DEFAULT_PIN_WL_DATA_IN 24u
#endif

// gpio (irq) pin for the irq line from the cyw43 chip
#ifndef CYW43_DEFAULT_PIN_WL_HOST_WAKE
#define CYW43_DEFAULT_PIN_WL_HOST_WAKE 24u
#endif

// gpio pin for the spi clock line to the cyw43 chip
#ifndef CYW43_DEFAULT_PIN_WL_CLOCK
#define CYW43_DEFAULT_PIN_WL_CLOCK 29u
#endif

// gpio pin for the spi chip select to the cyw43 chip
#ifndef CYW43_DEFAULT_PIN_WL_CS
#define CYW43_DEFAULT_PIN_WL_CS 25u
#endif

#endif

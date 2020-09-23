#include "stm32h7xx_hal.h"
#ifndef __LIS3DH_H__

#define LIS3DH_DEVICE_ADDRESS 0b0011000 << 1

#define LIS3DH_ADDR_AUTO_INC 0x80

#define LIS3DH_STATUS_REG_AUX 0x07
// 0b10000000 - 321OR - Overrun occurred on axis 1, 2 or 3 (data replaced)
// 0b01000000 - 3OR - " axis 3
// 0b00100000 - 2OR - " axis 2
// 0b00010000 - 1OR - " axis 1
// 0b00001000 - 321DA - New data available for axis 1, 2 or 3
// 0b00000100 - 3DA - " axis 3
// 0b00000010 - 2DA - " axis 2
// 0b00000001 - 1DA - " axis 1
#define LIS3DH_OUT_ADC1_L 0x08 // Low byte of ADC1, followed by high byte in 0x09
#define LIS3DH_OUT_ADC2_L 0x0A // Low byte of ADC2, followed by high byte in 0x0B
#define LIS3DH_OUT_ADC3_L 0x0C // Low byte of ADC3, followed by high byte in 0x0D
#define LIS3DH_WHO_AM_I 0x0F // Should be 0b00110011
#define LIS3DH_CTRL_REG0 0x1E
// 0b10000000 - SDO_PU_DISC - Disconnect SDO/SA0 pull-up
// Unused bits MUST be set at default values: 0bX0010000
#define LIS3DH_TEMP_CFG_REG 0x1F
// 0b10000000 - ADC_EN - Enable ADC
// 0b01000000 - TEMP_EN - Enable temperature sensor
#define LIS3DH_CTRL_REG1 0x20
// 0b00001000 - LPen - Low power enable
// 0b00000100 - Zen - Z-axis enable
// 0b00000010 - Yen - Y-axis enable
// 0b00000001 - Xen - X-axis enable
// 0b11110000 - ODR[3:0] - Data rate selection, 0000 = power down (default), others:
// 0b0001 - 1Hz
// 0b0010 - 10Hz
// 0b0011 - 25Hz
// 0b0100 - 50Hz
// 0b0101 - 100Hz
// 0b0110 - 200Hz
// 0b0111 - 400Hz
// 0b1000 - 1600Hz
// 0b1001 - 1344Hz (normal) or 5376Hz (low power?)
#define LIS3DH_CTRL_REG2 0x21
// 0b11000000 - HPM[1:0] - High-pass filter selection. 00 = Normal, 01 = Reference signal for filtering, 0b10 = Normal, 0b11 = Autoreset on interrupt event
// 0b00110000 - HPCF[2:1] - High-pass cutoff freq
// 0b00001000 - FDS - Filtered data selection
// 0b00000100 - HPCLICK - High-pass filter enabled for CLICK function
// 0b00000010 - HP_IA2 - AOI function on interrupt 2
// 0b00000001 - HP_IA1 - AOI function on interrupt 1
#define LIS3DH_CTRL_REG3 0x22
// 0b10000000 - I1_CLICK - Click interrupt on INT1
// 0b01000000 - I1_IA1
// 0b00100000 - I1_IA2
// 0b00010000 - I1_ZYXDA
// 0b00001000 - I1_321DA
// 0b00000100 - I1_WTM
// 0b00000010 - I1_OVERRUN
#define LIS3DH_CTRL_REG4 0x23
// 0b10000000 - BDU
// 0b01000000 - BLE
// 0b00110000 - FS[1:0] - 00 +-2g. 01 +-4g. 10 +-8g. +-16g
// 0b00001000 - HR - High Resolution mode
// 0b00000110 - ST[1:0] - Self test enable
// 0b00000001 - SIM - SPI serial interface mode
#define LIS3DH_CTRL_REG5 0x24
// 0b10000000 - BOOT
// 0b01000000 - FIFO_EN
// 0b00001000 - LIR_INT1
// 0b00000100 - D4D_INT1
// 0b00000010 - LIR_INT2
// 0b00000001 - D4D_INT2
#define LIS3DH_CTRL_REG6 0x25
// 0b10000000 - I2_CLICK
// 0b01000000 - I2_IA1
// 0b00100000 - I2_IA2
// 0b00010000 - I2_BOOT
// 0b00001000 - I2_ACT
// 0b00000010 - INT_POLARITY
#define LIS3DH_REFERENCE 0x26
// 0b11111111 - REF[7:0] - Reference value for interrupt generation
#define LIS3DH_STATUS_REG 0x27
// 0b10000000 - ZYXOR - ZYX-axis data overrun
// 0b01000000 - ZOR - Z-axis data overrun
// 0b00100000 - YOR - Y-axis data overrun
// 0b00010000 - XOR - X-axis data overrun
// 0b00001000 - ZYXDA - ZYX new data available
// 0b00000100 - ZDA - Z-axis new data available
// 0b00000010 - YDA - Y-axis new data available
// 0b00000001 - XDA - X-axis new data available
#define LIS3DH_OUT_X_L 0x28 // Low byte of X, followed by high byte in 0x29
#define LIS3DH_OUT_Y_L 0x2A // Low byte of Y, followed by high byte in 0x2B
#define LIS3DH_OUT_Z_L 0x2C // Low byte of Z, followed by high byte in 0x2D
#define LIS3DH_FIFO_CTRL_REG 0x2E
// 0b11000000 - FM[1:0] - FIFO mode selection
// 0b00100000 - TR - Trigger selection
// 0b00010000 - FTH[4:0]
#define LIS3DH_SRC_REG 0x2F
// 0b10000000 - WTM - bit set high when FIFO content exceeds watermark
// 0b01000000 - OVRN - set high when FIFO is full (32 unread samples)
// 0b00100000 - EMPTY - set when all FIFO samples are read
// 0b00011111 - FSS[4:0] - number of unread samples
#define LIS3DH_INT1_CFG 0x30
// 0b10000000 - AOI - And/Or combo of interrupt events
// 0b01000000 - 6D - 6 direction detection enabled
// 0b00100000 - ZHIE - Enable Z high interrupt
// 0b00010000 - ZLIE - Enable Z low interrupt
// 0b00001000 - YHIE - Enable Y high interrupt
// 0b00000100 - YLIE - Enable Y low interrupt
// 0b00000010 - XHIE - Enable X high interrupt
// 0b00000001 - XLIE - Enable X low interrupt
#define LIS3DH_INT1_SRC 0x31 // Interrupt source (Read Only)
// 0b01000000 - IA - Interrupt active
// 0b00100000 - ZH - Z high
// 0b00010000 - ZL - Z low
// 0b00001000 - YH - Y high
// 0b00000100 - YL - Y low
// 0b00000010 - XH - X high
// 0b00000001 - XL - L high
#define LIS3DH_INT1_THS 0x32
// 0b11111111 - THS[6:0] - Threshold value
#define LIS3DH_INT1_DURATION 0x33
// 0b11111111 - DUR[6:0] - Duration value
#define LIS3DH_INT2_CFG 0x34
// 0b10000000 - AOI - And/Or combo of interrupt events
// 0b01000000 - 6D - 6 direction detection enabled
// 0b00100000 - ZHIE - Enable Z high interrupt
// 0b00010000 - ZLIE - Enable Z low interrupt
// 0b00001000 - YHIE - Enable Y high interrupt
// 0b00000100 - YLIE - Enable Y low interrupt
// 0b00000010 - XHIE - Enable X high interrupt
// 0b00000001 - XLIE - Enable X low interrupt
#define LIS3DH_INT2_SRC 0x35
// 0b01000000 - IA - Interrupt active
// 0b00100000 - ZH - Z high
// 0b00010000 - ZL - Z low
// 0b00001000 - YH - Y high
// 0b00000100 - YL - Y low
// 0b00000010 - XH - X high
// 0b00000001 - XL - L high
#define LIS3DH_INT2_THS 0x36
// 0b11111111 - THS[6:0] - Threshold value
#define LIS3DH_INT2_DURATION 0x37
// 0b11111111 - DUR[6:0] - Duration value
#define LIS3DH_CLICK_CFG 0x38
// 0b00100000 - ZD - Enable double click on Z-axis
// 0b00010000 - ZS - Enable single click on Z-axis
// 0b00001000 - YD - Enable double click on Y-axis
// 0b00000100 - YS - Enable single click on Y-axis
// 0b00000010 - XD - Enable double click on X-axis
// 0b00000001 - XS - Enable single click on X-axis
#define LIS3DH_CLICK_SRC 0x39
// 0b01000000 - IA - Interrupt active
// 0b00100000 - DCLICK - Double-click enable
// 0000010000 - SCLICK - Single-click enable
// 0b00001000 - Sign - Click sign
// 0b00000100 - Z - Z click detection
// 0b00000010 - Y - Y click detection
// 0b00000001 - X - X click detection
#define LIS3DH_CLICK_THS 0x3A
// 0b10000000 - LIR_Click - 0 keep interrupt high for latency duration
// 0b01111111 - THS[6:0] - Click threshold
#define LIS3DH_TIME_LIMIT 0x3B
// 0b01111111 - TLI[6:0] - Click time limit
#define LIS3DH_TIME_LATENCY 0x3C
// 0b11111111 - TLA[7:0] - Click time latency
#define LIS3DH_TIME_WINDOW 0x3D
// 0b11111111 - TW[7:0] - Click time window
#define LIS3DH_ACT_THS 0x3F
// 0b01111111 - ACTH[6:0] - Sleep-to-wake, return-to-sleep activation threshold (low-power mode)
#define LIS3DH_ACT_DUR 0x3F
// 0b11111111 - ACTD[7:0] - Sleep-to-wake, return-to-sleep duration


#ifdef __cplusplus
extern "C" {
#endif

void lis3dh_init(I2C_HandleTypeDef *i2c_port);

#ifdef __cplusplus
}
#endif

#endif
/*****************************END OF FILE****/

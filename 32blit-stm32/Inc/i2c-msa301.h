#include "stm32h7xx_hal.h"
#ifndef __MSA301_H__


//device defines 

#define MSA301_DEVICE_ADDRESS       0x26 << 1

#define MSA301_ID_REGISTER          0x01

#define MSA301_CONTROL2_REGISTER            0x11
#define MSA301_CONTROL2_POWR_MODE_NORMAL    ( 0b00 << 6)
#define MSA301_CONTROL2_POWR_MODE_LOW       ( 0b01 << 6)
#define MSA301_CONTROL2_POWR_MODE_SUSPEND   ( 0b10 << 6)

#define MSA301_SWAP_REGISTER        0x12

#define MSA301_CONTROL1_REGISTER    0x10
#define MSA301_CONTROL1_ODR_1000HZ  0b1010
#define MSA301_CONTROL1_ODR_500HZ   0b1001
#define MSA301_CONTROL1_ODR_250HZ   0b1000
#define MSA301_CONTROL1_ODR_125HZ   0b111
#define MSA301_CONTROL1_ODR_62HZ5   0b110
#define MSA301_CONTROL1_ODR_31HZ25  0b101
#define MSA301_CONTROL1_ODR_15HZ63  0b100
#define MSA301_CONTROL1_ODR_7HZ81   0b11
#define MSA301_CONTROL1_ODR_3HZ9    0b10

#define MSA301_X_ACCEL_RESISTER   0x02

#define MSA301_Y_ACCEL_RESISTER   0x04

#define MSA301_Z_ACCEL_RESISTER   0x06

#ifdef __cplusplus
extern "C" {
#endif


void msa301_init(I2C_HandleTypeDef *i2c_port, uint8_t power_mode , uint8_t low_power_bandwidth , uint8_t update_rate);

void msa301_get_accel( I2C_HandleTypeDef *i2c_port, int16_t *data_buffer); //pass in reference to accerometer data buffer/store of int16[3] type 

int16_t twos_comp( uint16_t value , uint8_t bits);

#ifdef __cplusplus
}
#endif

#endif
/*****************************END OF FILE****/

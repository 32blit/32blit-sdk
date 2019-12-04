#include "stm32h7xx_hal.h"
#ifndef __BQ24295_H__
#define __BQ24295_H__


//device defines 

#define BQ24295_DEVICE_ADDRESS       0x6B << 1

#define BQ24295_ID_REGISTER          0x0A
#define BQ24295_TIMER_REGISTER       0x05
#define BQ24295_THERMAL_REGISTER     0x06
#define BQ24295_OP_CONTROL_REGISTER  0x07
#define BQ24295_SYS_STATUS_REGISTER  0x08


bool bq24295_init(I2C_HandleTypeDef *i2c_port);
uint8_t bq24295_get_status(I2C_HandleTypeDef *i2c_port);
void bq24295_enable_shipping_mode(I2C_HandleTypeDef *i2c_port);


#endif
/*****************************END OF FILE****/

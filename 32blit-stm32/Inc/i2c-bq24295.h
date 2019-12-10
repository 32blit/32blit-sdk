#include "stm32h7xx_hal.h"
#ifndef __BQ24295_H__
#define __BQ24295_H__

#define BQ24295_DEVICE_ADDRESS        0x6B << 1
#define BQ24294_PART_NUMBER           0b110

#define BQ24295_CONTROL_REGISTER      0x00
#define BQ24295_POWER_CFG_REGISTER    0x01
#define BQ24295_CHARGE_CTRL_REGISTER  0x02
#define BQ24295_CHARGE_ICTRL_REGISTER 0x03
#define BQ24295_CHARGE_VCTRL_REGISTER 0x04
#define BQ24295_TIMER_REGISTER        0x05
#define BQ24295_THERMAL_REGISTER      0x06
#define BQ24295_OP_CONTROL_REGISTER   0x07
#define BQ24295_SYS_STATUS_REGISTER   0x08
#define BQ24295_SYS_FAULT_REGISTER    0x09
#define BQ24295_ID_REGISTER           0x0A

void _i2c_send_8(I2C_HandleTypeDef *i2c_port, uint8_t address, uint8_t reg, uint8_t data);
uint8_t _i2c_recv_8(I2C_HandleTypeDef *i2c_port, uint8_t address, uint8_t reg);

extern bool bq24295_init(I2C_HandleTypeDef *i2c_port);
extern uint8_t bq24295_get_status(I2C_HandleTypeDef *i2c_port);
extern uint8_t bq24295_get_fault(I2C_HandleTypeDef *i2c_port);
extern void bq24295_enable_shipping_mode(I2C_HandleTypeDef *i2c_port);

#endif
#ifndef __USERI2C_H__
#define __USERI2C_H__

#include "stm32h7xx_hal.h"
#include "engine/useri2c.hpp"

using namespace blit;

namespace i2cUser {
    void set_i2c_port(I2C_HandleTypeDef *i2c_handle);
    void i2c_receive(uint8_t address, uint8_t reg, uint8_t &result, uint8_t result_size);
    void i2c_send(uint8_t address, uint8_t reg, uint8_t &data, uint8_t data_size);
}

#endif
/*****************************END OF FILE****/

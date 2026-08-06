

#include "i2c-msa301.h"

static void _i2c_send_8(I2C_HandleTypeDef *i2c_port, uint16_t address, uint8_t reg, uint8_t data);

//functions
void msa301_init(I2C_HandleTypeDef *i2c_port, uint8_t power_mode, uint8_t low_power_bandwidth, uint8_t update_rate){
  _i2c_send_8(i2c_port, MSA301_DEVICE_ADDRESS, MSA301_CONTROL1_REGISTER, update_rate);
  _i2c_send_8(i2c_port, MSA301_DEVICE_ADDRESS, MSA301_CONTROL2_REGISTER, (power_mode | low_power_bandwidth));
  _i2c_send_8(i2c_port, MSA301_DEVICE_ADDRESS, MSA301_SWAP_REGISTER, 0b00000011); // Reverse Z axis polarity and X/Y swap
}

static void _i2c_send_8(I2C_HandleTypeDef *i2c_port, uint16_t address, uint8_t reg, uint8_t data){
  uint8_t data_buffer[2];
  data_buffer[0] = reg;
  data_buffer[1] = data;
  HAL_I2C_Master_Transmit(i2c_port, address, &data_buffer[0] , 2 ,HAL_TIMEOUT);
}

  /*****************************END OF FILE****/

#include "i2c-lis3dh.h"

static void _i2c_send_8(I2C_HandleTypeDef *i2c_port, uint16_t address, uint8_t reg, uint8_t data);

void lis3dh_init(I2C_HandleTypeDef *i2c_port){
  _i2c_send_8(i2c_port, LIS3DH_DEVICE_ADDRESS, LIS3DH_CTRL_REG5, 0b10000000); // Reset
  HAL_Delay(15);
  _i2c_send_8(i2c_port, LIS3DH_DEVICE_ADDRESS, LIS3DH_CTRL_REG1, 0b01000111); // Enable XYZ, 50Hz
  _i2c_send_8(i2c_port, LIS3DH_DEVICE_ADDRESS, LIS3DH_CTRL_REG4, 0b10001000); // Enable high resolution mode & block data update
}

static void _i2c_send_8(I2C_HandleTypeDef *i2c_port, uint16_t address, uint8_t reg, uint8_t data){
  uint8_t data_buffer[2];
  data_buffer[0] = reg;
  data_buffer[1] = data;
  HAL_I2C_Master_Transmit(i2c_port, address, &data_buffer[0] , 2 ,HAL_TIMEOUT);
}


#include "i2c-user.hpp"

namespace i2cUser {
  I2C_HandleTypeDef *user_i2c_port;

  void set_i2c_port(I2C_HandleTypeDef *i2c_port) {
    user_i2c_port = i2c_port;
  }

  void i2c_receive(uint8_t address, uint8_t reg, uint8_t &result, uint8_t result_size){
      HAL_I2C_Master_Transmit(user_i2c_port, address, &reg, 1, HAL_TIMEOUT); //set register pointer   
      HAL_Delay(1); //Find shorter delay?
      HAL_I2C_Master_Receive(user_i2c_port, address, &result, result_size, HAL_TIMEOUT); //read twoo bytes from register 
  }

  void i2c_send(uint8_t address, uint8_t reg, uint8_t &data, uint8_t data_size){
      HAL_I2C_Master_Transmit(user_i2c_port, address, &reg, 1, HAL_TIMEOUT);
      HAL_I2C_Master_Transmit(user_i2c_port, address, &data, data_size, HAL_TIMEOUT);
  }

}
  /*****************************END OF FILE****/
  
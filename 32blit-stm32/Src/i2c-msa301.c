

#include "i2c-msa301.h"

static void _i2c_send_8(I2C_HandleTypeDef *i2c_port, uint16_t address, uint8_t reg, uint8_t data);

//functions 
void msa301_init(I2C_HandleTypeDef *i2c_port, uint8_t power_mode, uint8_t low_power_bandwidth, uint8_t update_rate){
  _i2c_send_8(i2c_port, MSA301_DEVICE_ADDRESS, MSA301_CONTROL1_REGISTER, update_rate);
  _i2c_send_8(i2c_port, MSA301_DEVICE_ADDRESS, MSA301_CONTROL2_REGISTER, (power_mode | low_power_bandwidth));
  _i2c_send_8(i2c_port, MSA301_DEVICE_ADDRESS, MSA301_SWAP_REGISTER, 0b00001111); // Reverse X, Y and Z axis polarity (and X/Y swap also??)
}

void msa301_get_accel(I2C_HandleTypeDef *i2c_port, int16_t *data_buffer) {
  // send address of the x-accelerometer value register as
  // our start reading position
  uint8_t reg = MSA301_X_ACCEL_RESISTER;
  HAL_I2C_Master_Transmit(i2c_port, MSA301_DEVICE_ADDRESS, &reg, 1, HAL_TIMEOUT);

  // read out six bytes of data (two each for x, y, z)
  uint8_t b[6];
  HAL_I2C_Master_Receive(i2c_port, MSA301_DEVICE_ADDRESS, b, 6, HAL_TIMEOUT);

  // each axis is stored as a 14-bit two's complement signed
  // integer. bits [5:0] are in the high end of the first byte
  // and bits [13:6] are in the second byte
  *data_buffer++ = (((int8_t)b[1] << 8) | (b[0] << 2)) >> 2;
  *data_buffer++ = (((int8_t)b[3] << 8) | (b[2] << 2)) >> 2;
  *data_buffer++ = (((int8_t)b[5] << 8) | (b[4] << 2)) >> 2;
}

static uint16_t _i2c_receive_16(I2C_HandleTypeDef *i2c_port, uint16_t address, uint8_t reg ){
  uint8_t receive_buffer[2];
  uint16_t ret_16b;
  HAL_I2C_Master_Transmit(i2c_port, address, &reg, 1, HAL_TIMEOUT); //set register pointer
  HAL_Delay(1);
  HAL_I2C_Master_Receive(i2c_port, address, &receive_buffer[0], 2, HAL_TIMEOUT); //read twoo bytes from register
  
  ret_16b = (receive_buffer[0] << 8) + receive_buffer [1]; //combine MSB LSB

  return ret_16b;
}

static void _i2c_send_8(I2C_HandleTypeDef *i2c_port, uint16_t address, uint8_t reg, uint8_t data){
  uint8_t data_buffer[2];
  data_buffer[0] = reg;
  data_buffer[1] = data;
  HAL_I2C_Master_Transmit(i2c_port, address, &data_buffer[0] , 2 ,HAL_TIMEOUT);
}
  
  /*****************************END OF FILE****/
  
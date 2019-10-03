
//#include "stm32h7xx_hal.h"   //replace with stm32h750
#include "stm32h7xx_hal.h"
#ifndef __RGB5X5_H__
#define __RGB5X5_H__


//device defines 

#define RGB5X5_DEVICE_ADDRESS       0x74 << 1

#define RGB5X5_MODE_REGISTER 0x00
#define RGB5X5_FRAME_REGISTER 0x01
#define RGB5X5_AUTOPLAY1_REGISTER 0x02
#define RGB5X5_AUTOPLAY2_REGISTER 0x03
#define RGB5X5_BLINK_REGISTER 0x05
#define RGB5X5_AUDIOSYNC_REGISTER 0x06
#define RGB5X5_BREATH1_REGISTER 0x08
#define RGB5X5_BREATH2_REGISTER 0x09
#define RGB5X5_SHUTDOWN_REGISTER 0x0a
#define RGB5X5_GAIN_REGISTER 0x0b
#define RGB5X5_ADC_REGISTER 0x0c

#define RGB5X5_CONFIG_BANK 0x0b
#define RGB5X5_BANK_ADDRESS 0xfd

#define RGB5X5_PICTURE_MODE 0x00
#define RGB5X5_AUTOPLAY_MODE 0x08
#define RGB5X5_AUDIOPLAY_MODE 0x18

#define RGB5X5_ENABLE_OFFSET 0x00
#define RGB5X5_BLINK_OFFSET 0x12
#define RGB5X5_PWM_OFFSET 0x24


bool rgb5x5_init(I2C_HandleTypeDef *i2c_port);
void rgb5x5_rgb(I2C_HandleTypeDef *i2c_port, uint8_t r, uint8_t g, uint8_t b);

#endif
/*****************************END OF FILE****/

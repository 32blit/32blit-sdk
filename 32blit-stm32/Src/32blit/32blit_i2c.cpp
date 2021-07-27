// 32blit_i2c.cpp
//
// 32Blit Firmware I2C features
//

#include "32blit.h"
#include "32blit_battery.hpp"
#include "32blit_i2c.hpp"
#include "engine/running_average.hpp"

#include "i2c.h"
#include "i2c-msa301.h"
#include "i2c-lis3dh.h"
#include "i2c-bq24295.h"

using namespace blit;

//
// I2C states for the I2C state machine
//
enum I2CState {
  DELAY,
  STOPPED,

  SEND_ACL,
  RECV_ACL,
  PROC_ACL,

  SEND_BAT_STAT,
  RECV_BAT_STAT,
  SEND_BAT_FAULT,
  RECV_BAT_FAULT,
  PROC_BAT
};

//
// Constants
//
static uint16_t accel_address = LIS3DH_DEVICE_ADDRESS;

//
// Local variables for this file
//
static RunningAverage<float> accel_x(8);
static RunningAverage<float> accel_y(8);
static RunningAverage<float> accel_z(8);

static I2CState i2c_state = SEND_ACL;
static uint8_t i2c_buffer[6] = {0};
static uint8_t i2c_reg = 0;
static HAL_StatusTypeDef i2c_status = HAL_OK;
static uint32_t i2c_delay_until = 0;
static I2CState i2c_next_state = SEND_ACL;

//
// delay for a given number of ms and transition to another I2CState
//
void blit_i2c_delay(uint16_t ms, I2CState state) {
  i2c_delay_until = HAL_GetTick() + ms;
  i2c_next_state = state;
  i2c_state = DELAY;
}

//
// Initialize the accelerometer
//
void blit_init_accelerometer() {
  if(is_beta_unit){
    msa301_init(&hi2c4, MSA301_CONTROL2_POWR_MODE_NORMAL, 0x00, MSA301_CONTROL1_ODR_62HZ5);
    accel_address = MSA301_DEVICE_ADDRESS;
  } else {
    lis3dh_init(&hi2c4);
  }
}

//
// I2C state machine runner
//
void blit_i2c_tick() {
  if(i2c_state == STOPPED) {
    return;
  }
  if(i2c_state == DELAY) {
    if(HAL_GetTick() >= i2c_delay_until){
      i2c_state = i2c_next_state;
    }
  }
  if(HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY){
    return;
  }

  switch(i2c_state) {
    case STOPPED:
    case DELAY:
      break;
    case SEND_ACL:
      i2c_reg = is_beta_unit ? MSA301_X_ACCEL_RESISTER : (LIS3DH_OUT_X_L | LIS3DH_ADDR_AUTO_INC);
      i2c_status = HAL_I2C_Master_Transmit_IT(&hi2c4, accel_address, &i2c_reg, 1);
      if(i2c_status == HAL_OK){
        i2c_state = RECV_ACL;
      } else {
        blit_i2c_delay(16, SEND_ACL);
      }
      break;
    case RECV_ACL:
      i2c_status = HAL_I2C_Master_Receive_IT(&hi2c4, accel_address, i2c_buffer, 6);
      if(i2c_status == HAL_OK){
        i2c_state = PROC_ACL;
      } else {
        blit_i2c_delay(16, SEND_ACL);
      }
      break;
    case PROC_ACL:
      // LIS3DH & MSA301 - 12-bit left-justified
      accel_x.add(((int8_t)i2c_buffer[1] << 6) | (i2c_buffer[0] >> 2));
      accel_y.add(((int8_t)i2c_buffer[3] << 6) | (i2c_buffer[2] >> 2));
      accel_z.add(((int8_t)i2c_buffer[5] << 6) | (i2c_buffer[4] >> 2));

      if(is_beta_unit){
        blit::tilt = Vec3(
          accel_x.average(),
          accel_y.average(),
          -accel_z.average()
        );
      } else {
        blit::tilt = Vec3(
          -accel_x.average(),
          -accel_y.average(),
          -accel_z.average()
        );
      }

      blit::tilt.normalize();

      i2c_state = SEND_BAT_STAT;
      break;
    case SEND_BAT_STAT:
      i2c_reg = BQ24295_SYS_STATUS_REGISTER;
      HAL_I2C_Master_Transmit_IT(&hi2c4, BQ24295_DEVICE_ADDRESS, &i2c_reg, 1);
      i2c_state = RECV_BAT_STAT;
      break;
    case RECV_BAT_STAT:
      HAL_I2C_Master_Receive_IT(&hi2c4, BQ24295_DEVICE_ADDRESS, i2c_buffer, 1);
      i2c_state = SEND_BAT_FAULT;
      break;
    case SEND_BAT_FAULT:
      i2c_reg = BQ24295_SYS_FAULT_REGISTER;
      HAL_I2C_Master_Transmit_IT(&hi2c4, BQ24295_DEVICE_ADDRESS, &i2c_reg, 1);
      i2c_state = RECV_BAT_FAULT;
      break;
    case RECV_BAT_FAULT:
      HAL_I2C_Master_Receive_IT(&hi2c4, BQ24295_DEVICE_ADDRESS, i2c_buffer + 1, 1);
      i2c_state = PROC_BAT;
      break;
    case PROC_BAT:
      battery::update_status(i2c_buffer[0], i2c_buffer[1]);
      blit_i2c_delay(16, SEND_ACL);
      break;
  }
}

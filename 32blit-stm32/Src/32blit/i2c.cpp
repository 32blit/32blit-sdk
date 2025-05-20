// 32blit_i2c.cpp
//
// 32Blit Firmware I2C features
//

#include "32blit.h"
#include "32blit/battery.hpp"
#include "32blit/i2c.hpp"
#include "engine/api_private.hpp"
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
  PROC_BAT,

  SEND_REG_USER,
  DATA_USER,
  DONE_USER,
};

//
// Constants
//
static uint16_t accel_address = LIS3DH_DEVICE_ADDRESS;

//
// Local variables for this file
//
static RunningAverage<int16_t> accel_x(8);
static RunningAverage<int16_t> accel_y(8);
static RunningAverage<int16_t> accel_z(8);

static I2CState i2c_state = SEND_ACL;
static uint8_t i2c_buffer[6] = {0};
static uint8_t i2c_reg = 0;
static uint32_t i2c_delay_until = 0;
static I2CState i2c_next_state = SEND_ACL;

static uint8_t user_addr = 0, user_reg = 0;
static bool user_recv = false;
static uint8_t *user_buf = nullptr;
static uint16_t user_buf_len = 0;

//
// delay for a given number of ms and transition to another I2CState
//
static void blit_i2c_delay(uint16_t ms, I2CState state) {
  i2c_delay_until = HAL_GetTick() + ms;
  i2c_next_state = state;
  i2c_state = DELAY;
}

namespace i2c {

  //
  // Initialise i2c devices
  //
  void init() {
    // init accelerometer
    if(is_beta_unit){
      msa301_init(&hi2c4, MSA301_CONTROL2_POWR_MODE_NORMAL, 0x00, MSA301_CONTROL1_ODR_62HZ5);
      accel_address = MSA301_DEVICE_ADDRESS;
    } else {
      lis3dh_init(&hi2c4);
    }

    // init battery management
    bq24295_init(&hi2c4);
  }

  //
  // I2C state machine runner
  //
  void tick() {
    if(i2c_state == STOPPED) {
      return;
    }
    if(i2c_state == DELAY) {
      if(HAL_GetTick() >= i2c_delay_until){
        i2c_state = i2c_next_state;
      }
      return;
    }
    if(HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY || __HAL_I2C_GET_FLAG(&hi2c4, I2C_FLAG_BUSY)){
      return;
    }

    switch(i2c_state) {
      case STOPPED:
      case DELAY:
        break;
      case SEND_ACL:
        i2c_reg = is_beta_unit ? MSA301_X_ACCEL_RESISTER : (LIS3DH_OUT_X_L | LIS3DH_ADDR_AUTO_INC);
        HAL_I2C_Master_Transmit_IT(&hi2c4, accel_address, &i2c_reg, 1);
        i2c_state = RECV_ACL;
        break;
      case RECV_ACL:
        HAL_I2C_Master_Receive_IT(&hi2c4, accel_address, i2c_buffer, 6);
        i2c_state = PROC_ACL;
        break;
      case PROC_ACL:
        // LIS3DH - 12-bit left-justified
        // MSA301 - 14-bit left-justified
        // shift it down later so the average drops less bits
        accel_x.add(((int8_t)i2c_buffer[1] << 8) | (i2c_buffer[0]));
        accel_y.add(((int8_t)i2c_buffer[3] << 8) | (i2c_buffer[2]));
        accel_z.add(((int8_t)i2c_buffer[5] << 8) | (i2c_buffer[4]));

        blit::tilt = Vec3(
          -accel_x.average() >> 2,
          -accel_y.average() >> 2,
          -accel_z.average() >> 2
        );

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
        i2c_state = SEND_REG_USER;
        break;

      case SEND_REG_USER:
        if(user_addr) {
          HAL_I2C_Master_Transmit_IT(&hi2c4, user_addr << 1, &user_reg, 1);
          i2c_state = user_buf_len ? DATA_USER : DONE_USER;
        } else // skip user states
          blit_i2c_delay(16, SEND_ACL);
        break;

      case DATA_USER:
        if(user_recv)
          HAL_I2C_Master_Receive_IT(&hi2c4, user_addr << 1, user_buf, user_buf_len);
        else
          HAL_I2C_Master_Transmit_IT(&hi2c4, user_addr << 1, user_buf, user_buf_len);

        i2c_state = DONE_USER;
        break;

      case DONE_USER:
      {
        auto addr = user_addr;
        user_addr = 0;
        if(api_data.i2c_completed && !blit_user_code_disabled())
          api_data.i2c_completed(addr, user_reg, user_buf, user_buf_len);

        blit_i2c_delay(16, SEND_ACL);
        break;
      }
    }
  }

  bool user_send(uint8_t address, uint8_t reg, const uint8_t *data, uint16_t len) {
    if(user_addr)
      return false;

    user_addr = address;
    user_reg = reg;
    user_recv = false;
    user_buf = const_cast<uint8_t *>(data);
    user_buf_len = len;

    return true;
  }

  bool user_receive(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
    if(user_addr)
      return false;

    user_addr = address;
    user_reg = reg;
    user_recv = true;
    user_buf = data;
    user_buf_len = len;

    return true;
  }
}

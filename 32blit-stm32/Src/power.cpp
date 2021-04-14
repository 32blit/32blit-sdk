#include "power.hpp"

#include "stm32h7xx_hal.h"

#include "32blit.h"
#include "i2c.h"
#include "i2c-bq24295.h"

namespace power {
  // power state we're changing to
  enum class Target {
    IDLE,
    SLEEP,
    OFF
  };

  static uint32_t last_input_time = 0, sleep_fade_start = 0;
  float sleep_fade = 1.0f;
  const int sleep_inactivity_time = 120000; // ms before sleeping
  const int fade_out_time = 1000, auto_sleep_fade_out_time = 3000, fade_in_time = 500;

  static Target target = Target::IDLE;

  void update() {

    if(target == Target::IDLE) {
      // inactivity timeout
      if(HAL_GetTick() - last_input_time > sleep_inactivity_time) {
        target = Target::SLEEP;
        sleep_fade_start = HAL_GetTick();
      }
    } else if(target == Target::OFF && sleep_fade == 0.0f) {
      // fade to off complete
      bq24295_enable_shipping_mode(&hi2c4);
    }

    if(target != Target::IDLE) {
      // fade out (slower for auto-sleep)
      auto fade_time = target == Target::SLEEP ? auto_sleep_fade_out_time : fade_out_time;

      sleep_fade = std::max(0.0f, 1.0f - float(HAL_GetTick() - sleep_fade_start) / fade_time);
      blit_update_volume();
    } else if(sleep_fade < 1.0f) {
      // fade in
      sleep_fade = std::min(1.0f, float(HAL_GetTick() - sleep_fade_start) / fade_in_time);
      blit_update_volume();
    }
  }

  void update_active() {
    // fading out or done fading, fade back in
    if(target == Target::SLEEP) {
      target = Target::IDLE;
      sleep_fade_start = HAL_GetTick() - sleep_fade * fade_in_time;
    }

    last_input_time = HAL_GetTick();
  }

  void power_off() {
    target = Target::OFF;
    sleep_fade_start = HAL_GetTick();
  }
}

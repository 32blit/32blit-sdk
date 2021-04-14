#include "power.hpp"

#include "stm32h7xx_hal.h"

#include "32blit.h"

namespace power {
  // power state we're changing to
  enum class Target {
    IDLE,
    SLEEP
  };

  static uint32_t last_input_time = 0, sleep_fade_start = 0;
  float sleep_fade = 1.0f;
  const int sleep_inactivity_time = 120000; // ms before sleeping
  const int sleep_fade_out_time = 3000, sleep_fade_in_time = 500;

  static Target target = Target::IDLE;

  void update() {

    if(target == Target::IDLE) {
      // inactivity timeout
      if(HAL_GetTick() - last_input_time > sleep_inactivity_time) {
        target = Target::SLEEP;
        sleep_fade_start = HAL_GetTick();
      }
    }

    if(target != Target::IDLE) {
      // fade out
      sleep_fade = std::max(0.0f, 1.0f - float(HAL_GetTick() - sleep_fade_start) / sleep_fade_out_time);
      blit_update_volume();
    } else if(sleep_fade < 1.0f) {
      // fade in
      sleep_fade = std::min(1.0f, float(HAL_GetTick() - sleep_fade_start) / sleep_fade_in_time);
      blit_update_volume();
    }
  }

  void update_active() {
    // fading out or done fading, fade back in
    if(target == Target::SLEEP) {
      target = Target::IDLE;
      sleep_fade_start = HAL_GetTick() - sleep_fade * sleep_fade_in_time;
    }

    last_input_time = HAL_GetTick();
  }
}

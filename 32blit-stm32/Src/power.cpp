#include "power.hpp"

#include "stm32h7xx_hal.h"

#include "32blit.h"

namespace power {

  static uint32_t last_input_time = 0, sleep_fade_in_start = 0;
  float sleep_fade = 1.0f;
  const int sleep_inactivity_time = 120000; // ms before sleeping
  const int sleep_fade_out_time = 3000, sleep_fade_in_time = 500;

  void update() {
    if(HAL_GetTick() - last_input_time > sleep_inactivity_time) {
      sleep_fade = std::max(0.0f, 1.0f - float(HAL_GetTick() - last_input_time - sleep_inactivity_time) / sleep_fade_out_time);
      blit_update_volume();
    } else if(sleep_fade < 1.0f) {
      sleep_fade = std::min(1.0f, float(HAL_GetTick() - sleep_fade_in_start) / sleep_fade_in_time);
      blit_update_volume();
    }
  }

  void update_active() {
    // fading out or done fading, fade back in
    if(HAL_GetTick() - last_input_time > sleep_inactivity_time)
      sleep_fade_in_start = HAL_GetTick() - sleep_fade * sleep_fade_in_time;

    last_input_time = HAL_GetTick();
  }
}

#pragma once

namespace power {
  extern float sleep_fade; // screen/audio fade for sleep transitions

  void update();
  void update_active(); // called when input happens

  void power_off();
  bool is_off();
}

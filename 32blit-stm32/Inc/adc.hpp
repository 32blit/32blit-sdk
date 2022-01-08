#pragma once

namespace adc {
  enum class Value {
    joystick_x = 0,
    joystick_y,
    hack_left,
    hack_right,
    battery_charge
  };

  void init();

  void stop();

  uint16_t get_value(Value v);
}

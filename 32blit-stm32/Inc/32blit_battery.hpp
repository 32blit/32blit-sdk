// 32blit_battery.hpp
//
// 32Blit Firmware Battery features
//

#pragma once

#include "engine/running_average.hpp"

namespace battery {
  enum BatteryVbusStatus {
    VbusUnknown = 0b00,
    USBHost = 0b01,
    AdapterPort = 0b10,
    OnTheGo = 0b11,
  };

  enum BatteryChargeStatus {
    NotCharging = 0b00,
    PreCharging = 0b01,
    FastCharging = 0b10,
    ChargingComplete = 0b11,
  };

  struct BatteryInformation {
      BatteryChargeStatus charge_status;
      const char * charge_text;

      BatteryVbusStatus vbus_status;
      const char * vbus_text;

      float voltage;
  };

  // Battery information
  BatteryInformation get_info();

  // Return the current battery status
  BatteryChargeStatus get_charge_status();

  // Return the current battery vbus status
  BatteryVbusStatus get_vbus_status();

  // Update the battery status (called only from i2c processing)
  void update_status ( uint8_t status, uint8_t fault );

  // Update the battery charge value
  void update_charge ( float charge_value );
}

// 32blit_battery.h
//
// 32Blit Firmware Battery features
//

#pragma once

#include "engine/running_average.hpp"

struct BatteryInformation {
    const char * status_text;
    const char * vbus_text;
    float voltage;

    // low level status info
    uint8_t battery_status;
    uint8_t battery_fault;
};

enum BatteryStatus {
  NotCharging,
  PreCharging,
  FastCharging,
  ChargingComplete
};

class BatteryState final {
  public:
    BatteryState() : battery_average(8), battery(0.0f), battery_status(0), battery_fault(0) {}

  public:
    // Battery information
    BatteryInformation get_battery_info();

    // Return the current battery status
    BatteryStatus get_battery_status();

  public:
    // Update the battery status (called only from i2c processing)
    void update_battery_status ( uint8_t status, uint8_t fault );

    // Update the battery charge value
    void update_battery_charge ( float charge_value );

  private:
    const char * vbus_status_string();
    const char * charge_status_string();

  private:
    blit::RunningAverage<float> battery_average;
    float battery;
    uint8_t battery_status;
    uint8_t battery_fault;
};

extern BatteryState battery;

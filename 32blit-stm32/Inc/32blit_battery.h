// 32blit_battery.h
//
// 32Blit Firmware Battery features
//

#pragma once

#include "engine/running_average.hpp"

enum BatteryVbusStatus {
  VbusUnknown,
  USBHost,
  AdapterPort,
  OnTheGo,
};

enum BatteryChargeStatus {
  NotCharging,
  PreCharging,
  FastCharging,
  ChargingComplete,
};

struct BatteryInformation {
    BatteryChargeStatus charge_status;
    const char * charge_text;

    BatteryVbusStatus vbus_status;
    const char * vbus_text;

    float voltage;
};

class BatteryState final {
  public:
    BatteryState() : battery_average(8), battery(0.0f), battery_status(0), battery_fault(0) {}

  public:
    // Battery information
    BatteryInformation get_battery_info();

    // Return the current battery status
    BatteryChargeStatus get_battery_charge_status();

    // Return the current battery vbus status
    BatteryVbusStatus get_battery_vbus_status();

  public:
    // Update the battery status (called only from i2c processing)
    void update_battery_status ( uint8_t status, uint8_t fault );

    // Update the battery charge value
    void update_battery_charge ( float charge_value );

  private:
    const char * get_vbus_status_string();
    const char * get_charge_status_string();

  private:
    blit::RunningAverage<float> battery_average;
    float battery;
    uint8_t battery_status;
    uint8_t battery_fault;
};

extern BatteryState battery;

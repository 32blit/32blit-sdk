// 32blit_battery.cpp
//
// 32Blit Firmware Battery features
//
// 32Blit uses a BQ24295 to control battery/charging/etc. Use below programming guide for I2C status description/interpretation
//
// > Product page: https://www.ti.com/product/BQ24295
// > Programming Guide: https://www.ti.com/lit/ds/symlink/bq24295.pdf?ts=1609111702094&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FBQ24295
//

#include "32blit.h"
#include "32blit_battery.h"

using namespace blit;

namespace battery {

  // forward declare functions
  const char * get_vbus_status_string();
  const char * get_charge_status_string();

  // local variables
  static blit::RunningAverage<float> battery_average(8);
  static float battery = 0.0f;
  static uint8_t battery_status = 0;
  static uint8_t battery_fault = 0;

  //
  // Return information about the battery
  //
  BatteryInformation get_info() {
    BatteryInformation info;

    info.charge_status = get_charge_status();
    info.charge_text = get_charge_status_string();

    info.vbus_status = get_vbus_status();
    info.vbus_text = get_vbus_status_string();

    info.voltage = battery;

    return info;
  }

  //
  // Update battery status and fault (read using I2C)
  //
  void update_status ( uint8_t status, uint8_t fault ) {
    battery_status = status;
    battery_fault = fault;
  }

  //
  // Update the charge value
  //
  void update_charge ( float charge_value ) {
    battery_average.add(charge_value);
    battery = battery_average.average();
  }

  //
  // Return the current battery status
  //
  BatteryChargeStatus get_charge_status() {
    return (BatteryChargeStatus) ((battery_status >> 4) & 0b11);
  }

  //
  // Convert charge status to text representation
  //
  const char *get_charge_status_string() {
    switch(get_charge_status()){
      case BatteryChargeStatus::NotCharging:
        return "Nope";
      case BatteryChargeStatus::PreCharging:
        return "Pre";
      case BatteryChargeStatus::FastCharging:
        return "Fast";
      case BatteryChargeStatus::ChargingComplete:
        return "Done";
    }

    // unreachable
    return "";
  }

  //
  // Return the current VBUS status
  //
  BatteryVbusStatus get_vbus_status() {
    return (BatteryVbusStatus)(battery_status >> 6);
  }

  //
  // Convert VBUS status to text representation
  //
  const char *get_vbus_status_string() {
    switch(get_vbus_status()){
      case BatteryVbusStatus::VbusUnknown:
        return "Unknown";
      case BatteryVbusStatus::USBHost:
        return "USB Host";
      case BatteryVbusStatus::AdapterPort:
        return "Adapter";
      case BatteryVbusStatus::OnTheGo:
        return "OTG";
    }

    // unreachable
    return "";
  }
}

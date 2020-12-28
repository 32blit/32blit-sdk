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

//
// Convert VBUS status to text representation
//
const char *BatteryState::vbus_status_string() {
  switch(battery_status >> 6){
    case 0b00: // Unknown
      return "Unknown";
    case 0b01: // USB Host
      return "USB Host";
    case 0b10: // Adapter Port
      return "Adapter";
    case 0b11: // OTG
      return "OTG";
  }

  // unreachable
  return "";
}

//
// Convert charge status to text representation
//
const char *BatteryState::charge_status_string() {
  switch((battery_status >> 4) & 0b11){
    case 0b00: // Not Charging
      return "Nope";
    case 0b01: // Pre-charge
      return "Pre";
    case 0b10: // Fast Charging
      return "Fast";
    case 0b11: // Charge Done
      return "Done";
  }

  // unreachable
  return "";
}

//
// Return information about the battery
//
BatteryInformation BatteryState::get_battery_info() {
  BatteryInformation info;

  info.status_text = charge_status_string();
  info.vbus_text = vbus_status_string();
  info.voltage = battery;

  info.battery_status = battery_status;
  info.battery_fault = battery_fault;

  return info;
}

//
// Update battery status and fault (read using I2C)
//
void BatteryState::update_battery_status ( uint8_t status, uint8_t fault ) {
  battery_status = status;
  battery_fault = fault;
}

//
// Update the charge value
//
void BatteryState::update_battery_charge ( float charge_value ) {
  battery_average.add(charge_value);
  battery = battery_average.average();
}

//
// Return the current battery status
//
BatteryStatus BatteryState::get_battery_status() {
  auto status = (battery_status >> 4) & 0b11;
  switch(status){
    case 0b00: // Not charging
      return BatteryStatus::NotCharging;
    case 0b01: // Pre-charge
      return BatteryStatus::PreCharging;
    case 0b10: // Fast Charging
      return BatteryStatus::FastCharging;
    case 0b11: // Charge Done
      return BatteryStatus::ChargingComplete;
  }

  // Unreachable
  return (BatteryStatus)0;
}

BatteryState battery;

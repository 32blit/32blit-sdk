/* SystemMenuController.cpp
 * source file for Firmware menu
 *
 * When the user presses the MENU button, the system shows a menu on top of the content. This file
 * contains functions to help drawing this menu. The actual invoking code is still in 32blit.cpp
 * because that code has too many dependencies on the firmware code.
 */

#include "32blit.h"
#include "32blit_battery.hpp"
#include "32blit.hpp"
#include <algorithm>

using namespace blit;
using battery::BatteryInformation;
using battery::BatteryChargeStatus;
using battery::BatteryVbusStatus;

#include "SystemMenu/system_menu_controller.hpp"
#include "SystemMenu/firmware_menu.hpp"
#include "SystemMenu/battery_menu.hpp"
#include "SystemMenu/connectivity_menu.hpp"
#include "SystemMenu/about_menu.hpp"
#include "engine/version.hpp"

//
// Initialize the system menu controller
//
SystemMenuController::SystemMenuController() {
  current_menu = SystemMenus::Firmware;
}

//
// Preapre to show the system menus
//
void SystemMenuController::prepare() {
  set_menu(SystemMenus::Firmware);

  firmware_menu.prepare();
  battery_menu.prepare();
  connectivity_menu.prepare();
  about_menu.prepare();
}

//
// Call update on the currently selected menu
//
void SystemMenuController::update(uint32_t time) {
  switch (current_menu) {
  case SystemMenus::Firmware:
    firmware_menu.update(time);
    break;
  case SystemMenus::Battery:
    battery_menu.update(time);
    break;
  case SystemMenus::Connectivity:
    connectivity_menu.update(time);
    break;
  case SystemMenus::About:
    about_menu.update(time);
    break;
  }
}

//
// Render the currently selected menu
//
void SystemMenuController::render(uint32_t time) {
  switch (current_menu) {
  case SystemMenus::Firmware:
    firmware_menu.render();
    break;
  case SystemMenus::Battery:
    battery_menu.render();
    break;
  case SystemMenus::Connectivity:
    connectivity_menu.render();
    break;
  case SystemMenus::About:
    about_menu.render();
    break;
  }

  render_header_battery_status(time);
  render_footer_version(time);
}

//
// Render the battery bar
//
void SystemMenuController::render_header_battery_status(uint32_t time) {
  const int screen_width = blit::screen.bounds.w;
  const int screen_height = blit::screen.bounds.h;

  const Pen foreground_colour = get_menu_colour(10);
  const Pen bar_background_color = get_menu_colour(3);

  BatteryInformation bat = battery::get_info();

  screen.pen = foreground_colour;
  screen.text("bat", minimal_font, Point(screen_width - 80, 4));
  int battery_meter_width = 55;
  battery_meter_width = float(battery_meter_width) * (bat.voltage - 3.0f) / 1.1f;
  battery_meter_width = std::max(0, std::min(55, battery_meter_width));

  screen.pen = bar_background_color;
  screen.rectangle(Rect(screen_width - 60, 5, 55, 5));

  switch (bat.vbus_status) {
  case BatteryVbusStatus::VbusUnknown:
    screen.pen = get_menu_colour(5);
    break;
  case BatteryVbusStatus::USBHost:
    screen.pen = get_menu_colour(6);
    break;
  case BatteryVbusStatus::AdapterPort:
    screen.pen = get_menu_colour(6);
    break;
  case BatteryVbusStatus::OnTheGo:
    screen.pen = get_menu_colour(7);
    break;
  }
  screen.rectangle(Rect(screen_width - 60, 5, battery_meter_width, 5));

  auto battery_charge_status = bat.charge_status;
  if (battery_charge_status == BatteryChargeStatus::PreCharging || battery_charge_status == BatteryChargeStatus::FastCharging) {
    int battery_fill_width = (time / 500) % battery_meter_width;
    battery_fill_width = std::min(battery_meter_width, battery_fill_width);
    screen.pen = get_menu_colour(8);
    screen.rectangle(Rect(screen_width - 60, 5, battery_fill_width, 5));
  }
}

//
// Render the footer for the menu
// The current menu can override this if needed
//
void SystemMenuController::render_footer_version(uint32_t time) {
  const int screen_width = blit::screen.bounds.w;
  const int screen_height = blit::screen.bounds.h;

  screen.pen = get_menu_colour(10);

  // render the version string into the footer
  screen.text(get_version_string(), minimal_font, Point(screen_width - 5, screen_height - 11), true, TextAlign::top_right);
}

//
// Change which menu to display
//
void SystemMenuController::set_menu(SystemMenus menu) {
  current_menu = menu;
}

//
// Declare the instance of the system_menu controller
//
SystemMenuController system_menu;

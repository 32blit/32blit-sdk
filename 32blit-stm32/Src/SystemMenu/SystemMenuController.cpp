/* SystemMenuController.cpp
 * source file for Firmware menu
 * 
 * When the user presses the MENU button, the system shows a menu on top of the content. This file
 * contains functions to help drawing this menu. The actual invoking code is still in 32blit.cpp
 * because that code has too many dependencies on the firmware code.
 */

#include "32blit.h"
#include "32blit.hpp"
#include <algorithm>

using namespace blit;

#include "SystemMenuController.hpp"
#include "FirmwareMenu.hpp"
#include "BatteryMenu.hpp"
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
}

//
// Call update on the currently selected menu
//
void SystemMenuController::update(uint32_t time) {
    switch ( current_menu ) {
        case SystemMenus::Firmware:
            firmware_menu.update(time);
            break;
        case SystemMenus::Battery:
            battery_menu.update(time);
            break;
    }
}

//
// Render the currently selected menu
//
void SystemMenuController::render(uint32_t time) {
    switch ( current_menu ) {
        case SystemMenus::Firmware:
            firmware_menu.render();
            break;
        case SystemMenus::Battery:
            battery_menu.render();
            break;
    }

    render_header(time);
    render_footer(time);
}

//
// Render the header for the menu
// The current menu can override this if needed
//
void SystemMenuController::render_header(uint32_t time) {
    render_header_battery_status(time);
}

//
// Render the battery bar
//
void SystemMenuController::render_header_battery_status(uint32_t time) {
  const int screen_width = blit::screen.bounds.w;
  const int screen_height = blit::screen.bounds.h;

//   const Pen foreground_colour = get_menu_colour(10);
  const Pen bar_background_color = get_menu_colour(3);

  BatteryInformation bat = blit_get_battery_info();

  screen.text("bat", minimal_font, Point(screen_width - 80, 4));
  int battery_meter_width = 55;
  battery_meter_width = float(battery_meter_width) * (bat.voltage - 3.0f) / 1.1f;
  battery_meter_width = std::max(0, std::min(55, battery_meter_width));

  screen.pen = bar_background_color;
  screen.rectangle(Rect(screen_width - 60, 5, 55, 5));

  switch(bat.battery_status >> 6){
    case 0b00: // Unknown
        screen.pen = get_menu_colour(5);
        break;
    case 0b01: // USB Host
        screen.pen = get_menu_colour(6);
        break;
    case 0b10: // Adapter Port
        screen.pen = get_menu_colour(6);
        break;
    case 0b11: // OTG
        screen.pen = get_menu_colour(7);
        break;
  }
  screen.rectangle(Rect(screen_width - 60, 5, battery_meter_width, 5));
  uint8_t battery_charge_status = (bat.battery_status >> 4) & 0b11;
  if(battery_charge_status == 0b01 || battery_charge_status == 0b10){
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
void SystemMenuController::render_footer(uint32_t time) {
    const int screen_width = blit::screen.bounds.w;
    const int screen_height = blit::screen.bounds.h;

    screen.pen = get_menu_colour(10);

    switch ( current_menu ) {
        case SystemMenus::Firmware:
            firmware_menu.render_footer();
            break;
        case SystemMenus::Battery:
            battery_menu.render_footer();
            break;
    }

    // render the version string into the footer
    screen.text(get_version_string(), minimal_font, Point(screen_width - 5, screen_height - 11), true, TextAlign::top_right);
}

//
// Change which menu to display
//
void SystemMenuController::set_menu ( SystemMenus menu ) {
    current_menu = menu;
}

//
// Declare the instance of the system_menu controller
//
SystemMenuController system_menu;

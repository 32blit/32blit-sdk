/* battery_menu.cpp
 * source file for Battery menu
 *
 * The battery menu is a sub menu to show battery information
 */

#include "32blit.h"
#include "32blit_battery.hpp"
#include "32blit.hpp"
#include "file.hpp"
#include "USBManager.h"
#include "i2c-bq24295.h"

using namespace blit;
using battery::BatteryInformation;
using battery::BatteryChargeStatus;
using battery::BatteryVbusStatus;

#include "battery_menu.hpp"
#include "system_menu_controller.hpp"

// To use the Item class without specifier
using blit::Menu;

//
// Menu items for the battery menu
//
enum MenuItem {
  CHARGE,
  VBUS,
  VOLTAGE,
  LAST_COUNT // leave me last pls
};

//
// Prepare to show the battery menu
//
void BatteryMenu::prepare() {
  background_colour = get_menu_colour(1);
  foreground_colour = get_menu_colour(2);
  bar_background_color = get_menu_colour(3);
  selected_item_background = get_menu_colour(4);
  header_background = get_menu_colour(9);
  header_foreground = get_menu_colour(10);

  display_rect.w = screen.bounds.w;
  display_rect.h = screen.bounds.h;
}

void BatteryMenu::render_item(const Item &item, int y, int index) const {
  Menu::render_item(item, y, index);

  const auto screen_width = screen.bounds.w;

  const int bar_margin = 2;
  const int bar_height = item_h - bar_margin * 2;
  const int bar_width = 75;
  int bar_x = screen_width - bar_width - item_padding_x;

  BatteryInformation bat = battery::get_info();

  switch (item.id) {
  case CHARGE:
    screen.text(bat.charge_text, minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    break;
  case VBUS:
    screen.text(bat.vbus_text, minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    break;
  case VOLTAGE:
    {
      char buf[100];
      snprintf(buf, 100, "%i.%i", (int)bat.voltage, (int)((bat.voltage - (int)bat.voltage) * 10.0f));
      screen.text(buf, minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    }
    break;
  default:
    screen.pen = foreground_colour;
    screen.text("Press A", minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    break;
  }
}

//
// Render the footer for the menu
//
void BatteryMenu::render_footer(int x, int y, int w) {
  Menu::render_footer(x, y, w);
  screen.pen = get_menu_colour(10);
  screen.text("B: Back", minimal_font, Point(x + 5, y + 5));
}

//
// Update backlight and volume by checking if keys were pressed
//
void BatteryMenu::update_menu(uint32_t time) {
  if (blit::buttons.released & blit::Button::DPAD_LEFT) {
    system_menu.set_menu(SystemMenus::Firmware);
  }
  if (blit::buttons.released & blit::Button::B) {
    system_menu.set_menu(SystemMenus::Firmware);
  }
}

//
// Menu items in the firmware menu
//
static Menu::Item battery_menu_items[]{
  {CHARGE, "Charge status"},
  {VBUS, "VBUS"},
  {VOLTAGE, "Voltage"},
};

//
// The actual firmware menu
//
BatteryMenu battery_menu("Battery Menu", battery_menu_items, MenuItem::LAST_COUNT);

/* battery_menu.cpp
 * source file for Battery menu
 *
 * The battery menu is a sub menu to show battery information
 */

#include "32blit.h"
#include "32blit_battery.hpp"
#include "32blit.hpp"

using namespace blit;
using battery::BatteryInformation;
using battery::BatteryChargeStatus;
using battery::BatteryVbusStatus;

#include "SystemMenu/battery_menu.hpp"
#include "SystemMenu/system_menu_controller.hpp"

//
// Menu items for the battery menu
//
enum MenuItem {
  CHARGE,
  VBUS,
  VOLTAGE,
};

static const Menu::Item menu_items[]{
  {CHARGE, "Charge status"},
  {VBUS, "VBUS"},
  {VOLTAGE, "Voltage"},
};

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
// The actual firmware menu
//
BatteryMenu battery_menu("Battery", menu_items, std::size(menu_items));

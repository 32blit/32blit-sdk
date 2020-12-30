/* about_menu.cpp
 * source file for About menu
 *
 * "The" Credits
 */

#include <array>

#include "32blit.h"
#include "32blit.hpp"

using namespace blit;

#include "SystemMenu/about_menu.hpp"
#include "SystemMenu/system_menu_controller.hpp"

#include "engine/version.hpp"

//
// Menu items for the battery menu
//
enum MenuItem {
  FIRMWARE_VERSION,
  FIRMWARE_DATE,
  BLIT_DEVICE_TYPE,
};

static Menu::Item menu_items[]{
  { FIRMWARE_VERSION, "Version" },
  { FIRMWARE_DATE, "Date" },
  { Menu::Separator, nullptr },
  { BLIT_DEVICE_TYPE, "Device" },
};

void AboutMenu::render_item(const Item &item, int y, int index) const {
  Menu::render_item(item, y, index);

  const auto screen_width = screen.bounds.w;

  const int bar_margin = 2;
  const int bar_height = item_h - bar_margin * 2;
  const int bar_width = 75;
  int bar_x = screen_width - bar_width - item_padding_x;

  switch (item.id) {
  case FIRMWARE_VERSION:
    screen.text(get_version_string(), minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    break;
  case FIRMWARE_DATE:
    screen.text(get_build_date(), minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    break;
  case BLIT_DEVICE_TYPE:
    if(is_beta_unit) {
      screen.text("Beta unit", minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    }
    else {
      screen.text("Retail unit", minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
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
AboutMenu about_menu("About 32blit", menu_items, std::size(menu_items));

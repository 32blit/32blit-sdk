/* about_menu.cpp
 * source file for About menu
 *
 * "The" Credits
 */

#include <array>

#include "32blit.h"
#include "32blit.hpp"
#include "ff.h"

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
  SD_CARD
};

static const Menu::Item menu_items[]{
  { FIRMWARE_VERSION, "Version" },
  { FIRMWARE_DATE, "Date" },
  { Menu::Separator, nullptr },
  { BLIT_DEVICE_TYPE, "Device" },
  { SD_CARD, "SD Card"}
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
    } else {
      screen.text("Retail unit", minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    }
    break;
  case SD_CARD:
    if(blit_sd_mounted()) {
      FATFS *fs;
      DWORD free_clusters;
      char buf[100];

      auto res = f_getfree("", &free_clusters, &fs);

      if(res == 0) {
        // assuming 512b sectors
        uint32_t total_mb = ((fs->n_fatent - 2) * fs->csize) / 2048;
        uint32_t free_mb = (free_clusters * fs->csize) / 2048;

        snprintf(buf, 100, "%lu/%lu MB free", free_mb, total_mb);
      } else
        snprintf(buf, 100, "Unknown %i", res);

      screen.text(buf, minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    } else if(blit_sd_detected())
      screen.text("Not mounted", minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    else
      screen.text("Not inserted", minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
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

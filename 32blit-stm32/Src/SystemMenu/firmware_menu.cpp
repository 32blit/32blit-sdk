/* firmware_menu.cpp
 * source file for Firmware menu
 *
 * The firmware menu is the main menu displayed when the user presses the MENU button.
 * It is rendered on top of the current content that is shown on the screen.
 * It can be used to adjust various system settings or to show information
 * about the battery.
 */

#include "32blit.h"
#include "32blit.hpp"
#include "file.hpp"
#include "USBManager.h"
#include "i2c-bq24295.h"

using namespace blit;

#include "firmware_menu.hpp"
#include "system_menu_controller.hpp"

//
// Some declarations from 32blit.cpp that we need to do system things
//
extern USBManager g_usbManager;
extern bool take_screenshot;
void DFUBoot(void);
void blit_update_volume();

//
// To use the Item class without specifier
//
using blit::Menu;

//
// Menu items for the firmware menu
//
enum MenuItem {
  BACKLIGHT,
  VOLUME,
  SCREENSHOT,
  DFU,
  SHIPPING,
  SWITCH_EXE,
  STORAGE,
  SEPARATOR1,
  BATTERY_INFO,
  SEPARATOR2,
  ABOUT,
  LAST_COUNT // leave me last pls
};

static Menu::Item firmware_menu_items[] {
    {BACKLIGHT, "Backlight"},
    {VOLUME, "Volume"},
    {SCREENSHOT, "Take Screenshot"},
    {DFU, "DFU Mode"},
    {SHIPPING, "Power Off"},
    {SWITCH_EXE, ""}, // label depends on if a game is running
    {STORAGE, "Storage Mode"},
    {SEPARATOR1,nullptr},
    {BATTERY_INFO, "Battery info >"},
    {SEPARATOR2,nullptr},
    {ABOUT,"About 32blit >"},
};

//
// Prepare to show the firmware menu
//
void FirmwareMenu::prepare() {
  // slightly nasty dynamic label
  const_cast<Item *>(items)[SWITCH_EXE].label = blit_user_code_running() ? "Exit Game" : "Launch Game";

  background_colour = get_menu_colour(1);
  foreground_colour = get_menu_colour(2);
  bar_background_color = get_menu_colour(3);
  selected_item_background = get_menu_colour(4);
  header_background = get_menu_colour(9);
  header_foreground = get_menu_colour(10);
  bar_highlight_color = get_menu_colour(11);

  display_rect.w = screen.bounds.w;
  display_rect.h = screen.bounds.h;
}

void FirmwareMenu::draw_slider(Point pos, int width, float value, Pen colour) const {
  const int bar_margin = 2;
  const int bar_height = item_h - bar_margin * 2;

  screen.pen = bar_background_color;
  screen.rectangle(Rect(pos, Size(width, bar_height)));
  screen.pen = colour;
  screen.rectangle(Rect(pos, Size(width * value, bar_height)));
}

void FirmwareMenu::render_item(const Item &item, int y, int index) const {
  Menu::render_item(item, y, index);

  const auto screen_width = screen.bounds.w;

  const int bar_margin = 2;
  const int bar_height = item_h - bar_margin * 2;
  const int bar_width = 75;
  int bar_x = screen_width - bar_width - item_padding_x;

  switch (item.id) {
  case BACKLIGHT:
    draw_slider(Point(bar_x, y + bar_margin), bar_width, persist.backlight, foreground_colour);
    break;
  case VOLUME:
    if (persist.is_muted)
    {
      screen.pen = bar_highlight_color;
      screen.text("Muted", minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
    }
    else
    {
      draw_slider(Point(bar_x, y + bar_margin), bar_width, persist.volume, foreground_colour);
    }
    break;
  case STORAGE:
    screen.pen = foreground_colour;
    const char *label;
    if (num_open_files)
      label = "Files Open";
    else if (g_usbManager.GetType() == USBManager::usbtMSC)
      label = g_usbManager.GetStateName() + 4; // trim the "MSC "
    else
      label = "Disabled";

    screen.text(label, minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
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
void FirmwareMenu::render_footer(int x, int y, int w) {
  Menu::render_footer(x, y, w);
  screen.pen = get_menu_colour(10);
  screen.text("X: Mute", minimal_font, Point(x + 5, y + 5));
}

//
// For values that are changed using sliders, in addition to DPAD_LEFT and DPAD_RIGHT smooth
// changing, we support Y to set to 0, X to set to full and A and B to set set in 1/4 steps.
// The function gets a pointer to the value that is to be changed.
//
void FirmwareMenu::update_slider_item_value(float &value) {
  if (blit::buttons.released & blit::Button::DPAD_LEFT) {
    value -= 1.0f / 12.0f;
  }
  else if (blit::buttons.released & blit::Button::DPAD_RIGHT) {
    value += 1.0f / 12.0f;
  }
}

//
// Update backlight and volume by checking if keys were pressed
//
void FirmwareMenu::update_item(const Item &item) {
  if (item.id == BACKLIGHT) {
    update_slider_item_value(persist.backlight);
    persist.backlight = std::fmin(1.0f, std::fmax(0.0f, persist.backlight));
  }
  else if (item.id == VOLUME) {
    update_slider_item_value(persist.volume);
    persist.volume = std::fmin(1.0f, std::fmax(0.0f, persist.volume));
    blit_update_volume();
  }
  else if (item.id == BATTERY_INFO) {
    if (blit::buttons.released & blit::Button::DPAD_RIGHT)
    {
      system_menu.set_menu(SystemMenus::Battery);
    }
  }
  else if (item.id == ABOUT) {
    if (blit::buttons.released & blit::Button::DPAD_RIGHT)
    {
      system_menu.set_menu(SystemMenus::About);
    }
  }
}

//
// Update the entire menu
//
void FirmwareMenu::update_menu(uint32_t time) {
  if (blit::buttons.released & blit::Button::X) {
    persist.is_muted = !persist.is_muted;
    blit_update_volume();
  }
}

//
// The 'A' button was clicked on a menu item
//
void FirmwareMenu::item_activated(const Item &item) {
  switch (item.id) {
  case SCREENSHOT:
    take_screenshot = true;
    break;
  case DFU:
    DFUBoot();
    break;
  case SHIPPING:
    bq24295_enable_shipping_mode(&hi2c4);
    break;
  case BATTERY_INFO:
    system_menu.set_menu(SystemMenus::Battery);
    break;
  case ABOUT:
    system_menu.set_menu(SystemMenus::About);
    break;
  case SWITCH_EXE:
    blit_switch_execution(persist.last_game_offset, false);
    break;
  case STORAGE:
    // switch back manually if not mounted
    if (g_usbManager.GetState() == USBManager::usbsMSCInititalising)
      g_usbManager.SetType(USBManager::usbtCDC);
    else if (num_open_files == 0)
      g_usbManager.SetType(USBManager::usbtMSC);
    break;
  }
}

//
// The actual firmware menu
//
FirmwareMenu firmware_menu("System Menu", firmware_menu_items, MenuItem::LAST_COUNT);

/* BatteryMenu.cpp
 * source file for Battery menu
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

#include "BatteryMenu.hpp"
#include "SystemMenuController.hpp"

//
// To use the Item class without specifier
//
using blit::Menu;

//
// Menu items for the firmware menu
//
enum MenuItem {
    CHARGE,
    VBUS,
    VOLTAGE,
    LAST_COUNT // leave me last pls
};

//
// Prepare to show the firmware menu
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
  
    BatteryInformation bat = blit_get_battery_info();

    switch(item.id) {
      case CHARGE:
        screen.text(bat.status_text, minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
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
void BatteryMenu::render_footer() {
    const int screen_width = blit::screen.bounds.w;
    const int screen_height = blit::screen.bounds.h;
    screen.text("B: Back", minimal_font, Point(5, screen_height - 11));
}

//
// Update backlight and volume by checking if keys were pressed
//
void BatteryMenu::update_item(const Item &item) {
    // if(item.id == BACKLIGHT) {
    //   update_slider_item_value(persist.backlight);
    //   persist.backlight = std::fmin(1.0f, std::fmax(0.0f, persist.backlight));
    // } else if(item.id == VOLUME) {
    //   update_slider_item_value(persist.volume);
    //   persist.volume = std::fmin(1.0f, std::fmax(0.0f, persist.volume));
    //   blit_update_volume();
    // }
    if(blit::buttons & blit::Button::DPAD_LEFT) {
        system_menu.set_menu(SystemMenus::Firmware);
    }
    if(blit::buttons & blit::Button::B) {
        system_menu.set_menu(SystemMenus::Firmware);
    }
}

//
// The 'A' button was clicked on a menu item
//
void BatteryMenu::item_activated(const Item &item) {
    // switch(item.id) {
    //   case SCREENSHOT:
    //     take_screenshot = true;
    //     break;
    //   case DFU:
    //     DFUBoot();
    //     break;
    //   case SHIPPING:
    //     bq24295_enable_shipping_mode(&hi2c4);
    //     break;
    //   case SWITCH_EXE:
    //     blit_switch_execution(persist.last_game_offset, false);
    //     break;
    //   case STORAGE:
    //     // switch back manually if not mounted
    //     if(g_usbManager.GetState() == USBManager::usbsMSCInititalising)
    //       g_usbManager.SetType(USBManager::usbtCDC);
    //     else if(num_open_files == 0)
    //       g_usbManager.SetType(USBManager::usbtMSC);
    //     break;
    // }
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

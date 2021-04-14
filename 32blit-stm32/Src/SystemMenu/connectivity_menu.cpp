/* connectivity_menu.cpp
 * source file for Coonectivity menu
 *
 * The connectivity menu is a sub menu to show connectivity options
 */

#include "32blit.h"
#include "32blit.hpp"
#include "file.hpp"
#include "USBManager.h"
#include "multiplayer.hpp"

using namespace blit;

#include "SystemMenu/connectivity_menu.hpp"
#include "SystemMenu/system_menu_controller.hpp"

//
// Some declarations from 32blit.cpp that we need to do system things
//
extern USBManager g_usbManager;
void DFUBoot(void);

//
// Menu items for the battery menu
//
enum MenuItem {
  DFU,
  STORAGE,
};

static const Menu::Item menu_items[]{
  {DFU, "DFU Mode"},
  {STORAGE, "Storage Mode"},
};

void ConnectivityMenu::render_item(const Item &item, int y, int index) const {
  Menu::render_item(item, y, index);

  const auto screen_width = screen.bounds.w;

  switch (item.id) {
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

// The A button was pressed on a menu item
void ConnectivityMenu::item_activated(const Item &item) {
  switch (item.id) {
  case DFU:
    DFUBoot();
    break;
  case STORAGE:
    // switch back manually if not mounted
    if (g_usbManager.GetState() == USBManager::usbsMSCInititalising)
      g_usbManager.SetType(USBManager::usbtCDC);
    else if (num_open_files == 0 && !multiplayer::enabled)
      g_usbManager.SetType(USBManager::usbtMSC);
    break;
  }
}

ConnectivityMenu connectivity_menu("Connectivity", menu_items, std::size(menu_items));

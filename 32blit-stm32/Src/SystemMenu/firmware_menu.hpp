/* firmware_menu.hpp
 * header file for Firmware menu
 *
 * The firmware menu is the main menu displayed when the user presses the MENU button.
 * It is rendered on top of the current content that is shown on the screen.
 * It can be used to adjust various system settings or to show information
 * about the battery.
 */

#pragma once

#include "engine/menu.hpp"

class FirmwareMenu final : public blit::Menu {
public:
  using blit::Menu::Menu;

  // Prepare to show the firmware menu
  void prepare();

  void draw_slider(Point pos, int width, float value, Pen colour) const;

  void render_footer(int x, int y, int w) override;

protected:
  void render_item(const Item &item, int y, int index) const override;

  // Update menu items -- update backlight and volume by checking some keys
  void update_item(const Item &item) override;

  // Update entire menu
  void update_menu(uint32_t time) override;

  // The 'A' button was clicked on a menu item
  void item_activated(const Item &item) override;

private:
  // handle updating slider values
  void update_slider_item_value(float &value);

private:
  Pen bar_background_color;
  Pen bar_highlight_color;
};

extern FirmwareMenu firmware_menu;

/* BatteryMenu.hpp
 * header file for Battery menu
 *
 * The battery menu is a sub menu to show battery information
 */

#pragma once

#include "engine/menu.hpp"

class BatteryMenu final : public blit::Menu {
public:
  using blit::Menu::Menu;

  //
  // Prepare to show the battery menu
  //
  void prepare();

  void render_footer(int x, int y, int w) override;

protected:
  void render_item(const Item &item, int y, int index) const override;

  void update_menu(uint32_t time) override;

private:
  Pen bar_background_color;
};

extern BatteryMenu battery_menu;

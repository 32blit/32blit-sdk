/* battery_menu.hpp
 * header file for Battery menu
 *
 * The battery menu is a sub menu to show battery information
 */

#pragma once

#include "system_menu.hpp"
class BatteryMenu final : public SystemSubMenu {
public:
  using SystemSubMenu::SystemSubMenu;

protected:
  void render_item(const Item &item, int y, int index) const override;
};

extern BatteryMenu battery_menu;

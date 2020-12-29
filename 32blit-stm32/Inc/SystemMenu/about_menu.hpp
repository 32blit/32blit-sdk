/* about_menu.hpp
 * header file for About menu
 *
 * "The" Credits
 */

#pragma once

#include "system_menu.hpp"

class AboutMenu final : public SystemSubMenu {
public:
  using SystemSubMenu::SystemSubMenu;

protected:
  void render_item(const Item &item, int y, int index) const override;
};

extern AboutMenu about_menu;

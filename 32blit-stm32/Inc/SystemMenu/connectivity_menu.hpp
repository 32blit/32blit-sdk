/* connectivity_menu.hpp
 * header file for Connectivity menu
 *
 * The connectivity menu is a sub menu to show connectivity options
 */

#pragma once

#include "system_menu.hpp"

class ConnectivityMenu final : public SystemSubMenu {
public:
  using SystemSubMenu::SystemSubMenu;

protected:
  void item_activated(const Item &item) override;

  void render_item(const Item &item, int y, int index) const override;
};

extern ConnectivityMenu connectivity_menu;

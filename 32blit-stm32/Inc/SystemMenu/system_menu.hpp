#pragma once

#include "system_menu_controller.hpp"

#include "engine/menu.hpp"

class SystemMenu : public blit::Menu {
public:
  using blit::Menu::Menu;

  virtual void prepare() {
    background_colour = get_menu_colour(1);
    foreground_colour = get_menu_colour(2);
    bar_background_color = get_menu_colour(3);
    selected_item_background = get_menu_colour(4);
    header_background = get_menu_colour(9);
    header_foreground = get_menu_colour(10);

    display_rect.w = blit::screen.bounds.w;
    display_rect.h = blit::screen.bounds.h;
  }

protected:
  Pen bar_background_color;
};

class SystemSubMenu : public SystemMenu {
public:
  using SystemMenu::SystemMenu;

  void update_menu(uint32_t time) override {
    if (blit::buttons.released & blit::Button::DPAD_LEFT) {
      system_menu.set_menu(SystemMenus::Firmware);
    }
    if (blit::buttons.released & blit::Button::B) {
      system_menu.set_menu(SystemMenus::Firmware);
    }
  }

protected:
  void render_footer(int x, int y, int w) override {
    Menu::render_footer(x, y, w);
    screen.pen = get_menu_colour(10);
    screen.text("B: Back", minimal_font, Point(x + 5, y + 5));
  }
};

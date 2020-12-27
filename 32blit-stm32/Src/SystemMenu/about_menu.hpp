/* about_menu.hpp
 * header file for About menu
 *
 * "The" Credits
 */

#pragma once

#include "engine/menu.hpp"

class AboutMenu final : public blit::Menu {
public:
  using blit::Menu::Menu;

  void prepare();

  void reset_scrolling() {
    start_y = screen.bounds.h - footer_h;
    last_start = 0;
  }

  void render_menu() override;
  void update(uint32_t time) override;

protected:
  void render_footer(int x, int y, int w) override;

private:
  Pen bar_background_color;
  int start_y;
  uint32_t last_start;
};

extern AboutMenu about_menu;

#pragma once

#include "persistence.h"

Pen get_menu_colour(int index);

enum class SystemMenus {
  Firmware,
  Battery,
  Connectivity,
  About,
};

class SystemMenuController {
public:
  SystemMenuController();

public:
  void prepare();

  void render(uint32_t time);
  void update(uint32_t time);

private:
  void render_header_battery_status(uint32_t time);
  void render_footer_version(uint32_t time);

public:
  void set_menu(SystemMenus menu);

private:
  SystemMenus current_menu;
};

extern SystemMenuController system_menu;

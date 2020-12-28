/* about_menu.cpp
 * source file for About menu
 *
 * "The" Credits
 */

#include "32blit.h"
#include "32blit.hpp"
#include "file.hpp"
#include "USBManager.h"
#include "i2c-bq24295.h"

using namespace blit;

#include "about_menu.hpp"
#include "system_menu_controller.hpp"

#define SPEED 30

// To use the Item class without specifier
using blit::Menu;

static const char* credits[] = {
  "*32Blit",
  "A pimoroni production",
  "",
  "*Pimoroni Overlords",
  "Jon",
  "Guru",
  "",
  "*Pirate Captain",
  "Gadgetoid",
  "",
  "*Pirate Engineer",
  "Niko",
  "",
  "*Pixel Wrangler",
  "Sam (@s4m_ur4i)",
  "",
  "*The contributing crew",
  "(in alphabetical order)",
  "ahnlak",
  "ali1234",
  "andreban",
  "AndrewCapon",
  "Daft-Freak",
  "drisc",
  "illbewithee",
  "johnmccabe",
  "lenardg",
  "LordEidi",
  "lowfatcode",
  "lukeadlerhhl",
  "mikerr",
  "mylogon341",
  "ntwyman",
  "Ozzard",
  "Pharap",
  "shane-powell",
  "tinwhisker",
  "trollied",
  "voidberg",
  "ymauray",
  "zenodante",
  "",
  "*Kickstarted in 2019",
  "",
  "No parrots were harmed in",
  "the making of this product.",
  "",
  "*(c) MMXX",
  "." // credits ends with a string that starts with a dot!
};

// Prepare to show the about menu
void AboutMenu::prepare() {
  background_colour = get_menu_colour(1);
  foreground_colour = get_menu_colour(2);
  bar_background_color = get_menu_colour(3);
  selected_item_background = get_menu_colour(4);
  header_background = get_menu_colour(9);
  header_foreground = get_menu_colour(10);

  display_rect.w = screen.bounds.w;
  display_rect.h = screen.bounds.h;

  reset_scrolling();
}

void AboutMenu::render_menu() {
  Pen highlight = {  0, 255,   0};

  const auto screen_width = screen.bounds.w;
  auto next_index = 0;
  auto y = start_y;
  while ( credits[next_index][0] != '.') {
    if ( y >= header_h - 5 && y < screen.bounds.h - 5) {
      const char * text_to_render = credits[next_index];
      if ( text_to_render[0] == '*') {
        screen.pen = highlight;
        text_to_render++;
      } else {
        screen.pen = foreground_colour;
      }
      screen.text(text_to_render, minimal_font, Point(screen_width / 2, y), true, TextAlign::center_h);
    }

    next_index++;
    y += 10;
  }

  if ( y < 0 ) {
    reset_scrolling();
  }

  Menu::render_menu();
}

//
// Render the footer for the menu
//
void AboutMenu::render_footer(int x, int y, int w) {
  Menu::render_footer(x, y, w);
  screen.pen = get_menu_colour(10);
  screen.text("B: Back", minimal_font, Point(x + 5, y + 5));
}

//
// Update backlight and volume by checking if keys were pressed
//
void AboutMenu::update(uint32_t time) {
  if (blit::buttons.released & blit::Button::DPAD_LEFT) {
    system_menu.set_menu(SystemMenus::Firmware);
  }
  if (blit::buttons.released & blit::Button::B) {
    system_menu.set_menu(SystemMenus::Firmware);
  }

  if ( last_start == 0 ) {
    last_start = time;
  }

  start_y = screen.bounds.h - footer_h - ((time - last_start) / SPEED);
}

//
// Menu items in the firmware menu
//
static Menu::Item about_menu_items[] {
  {0, nullptr}
};

//
// The actual firmware menu
//
AboutMenu about_menu("About 32blit", about_menu_items, 0);

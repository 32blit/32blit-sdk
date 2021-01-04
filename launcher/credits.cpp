/* about_menu.cpp
 * source file for About menu
 *
 * "The" Credits
 */

#include "32blit.hpp"

using namespace blit;

#include "credits.hpp"

#define SPEED 40

namespace credits {

  const int cinematic_bar_height = 24;

  static Rect display_rect(0, 0, 0, 0);
  static Pen background_colour;
  static Pen foreground_colour;
  static Pen bar_colour(0,0,0);
  int start_y;
  uint32_t last_start;

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
    nullptr // credits ends with a nullptr
  };

  void reset_scrolling() {
    start_y = screen.bounds.h - cinematic_bar_height;
    last_start = 0;
  }


  // Prepare to show the about menu
  void prepare() {
    background_colour = Pen(30, 30, 50, 220);
    foreground_colour = { 255, 255, 255 };

    display_rect.w = screen.bounds.w;
    display_rect.h = screen.bounds.h;

    reset_scrolling();
  }

  void render() {
    Pen highlight = { 0, 255, 0 };

    // background
    screen.pen = background_colour;
    screen.rectangle(display_rect);

    // scrolling text
    const auto screen_width = screen.bounds.w;
    auto next_index = 0;
    auto y = start_y;
    while (credits[next_index] != nullptr) {
      if (y >= 0 && y < screen.bounds.h) {
        const char* text_to_render = credits[next_index];
        if (text_to_render[0] == '*') {
          screen.pen = highlight;
          text_to_render++;
        }
        else {
          screen.pen = foreground_colour;
        }
        screen.text(text_to_render, minimal_font, Point(screen_width / 2, y), true, TextAlign::center_h);
      }

      next_index++;
      y += 10;
    }

    screen.pen = bar_colour;
    screen.rectangle(Rect(0, 0, display_rect.w, cinematic_bar_height));
    screen.rectangle(Rect(0, display_rect.h - cinematic_bar_height, display_rect.w, cinematic_bar_height));

    if (y < 0) {
      reset_scrolling();
    }
  }

  //
  // Update backlight and volume by checking if keys were pressed
  //
  void update(uint32_t time) {
    if (last_start == 0) {
      last_start = time;
    }

    start_y = screen.bounds.h - cinematic_bar_height - ((time - last_start) / SPEED);
  }
}

/* about_menu.cpp
 * source file for About menu
 *
 * "The" Credits
 */

#include "32blit.hpp"

using namespace blit;

#include "assets.hpp"
#include "credits.hpp"
#include "contrib.hpp"
#include "theme.hpp"

// Speed of scrolling, ms per pixel
#define SPEED 40

#define ROW_HEIGHT 12

// External variables

namespace credits {
  static const Font launcher_font(font8x8);
  static const Font launcher_outline_font(outline_font9x10);

  enum class CreditRenderMode {
    Credits,
    SpecialThanks,
    Contributors
  };

  const int cinematic_bar_height = 24;

  static Rect display_rect(0, 0, 0, 0);
  static Pen background_colour;
  static Pen foreground_colour;
  static Pen highlight_colour;
  static Pen bar_colour(0,0,0);
  int start_y;
  uint32_t last_start_time;

  static const char* credits[] = {
    "*32Blit",
    "A Pimoroni production",
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
    "Rabid-inventor",
    "",
    "*Pixel Wrangler",
    "Sam (@s4m_ur4i)",
    "",
    "*Special thanks to",
    "%S", // code will render special thanks section here, %S is placeholder for this
    "",
    "*The contributing crew",
    "(in alphabetical order)",
    "%C", // code will render contributors section here, %C is placeholder
    "",
    "*Kickstarted in 2019",
    "",
    "No parrots were harmed in",
    "the making of this product.",
    "",
    "*(c) MMXXI",
    nullptr // credits ends with a nullptr
  };

  // Prepare to show the credits
  void prepare() {
    background_colour = Pen(30, 30, 50, 220);
    foreground_colour = theme.color_text;
    highlight_colour = theme.color_accent;

    display_rect.w = screen.bounds.w;
    display_rect.h = screen.bounds.h;

    reset_scrolling();
  }

  const Pen rainbow_colours[]{
    Pen(255,0,0),
    Pen(255,255,0),
    Pen(0,255,0),
    Pen(0,255,255),
    Pen(0,0,255),
    Pen(255,0,255)
  };
  static int colour_index = 0;
  const int number_of_colours = sizeof(rainbow_colours) / sizeof(Pen);

  void render() {
    // background
    screen.pen = background_colour;
    screen.rectangle(display_rect);

    // scrolling text
    const auto screen_width = screen.bounds.w;
    auto next_index = 0;
    auto y = start_y;

    auto mode = CreditRenderMode::Credits;
    auto contrib_index = 0;
    auto special_index = 0;

    while (credits[next_index] != nullptr) {
      const char* text_to_render = credits[next_index];
      screen.pen = foreground_colour;

      // handle special codes in the credits
      if (credits[next_index][0] == '*') {
        screen.pen = highlight_colour;
        text_to_render++;
      }
      else if (text_to_render[0] == '%') {
        if (text_to_render[1] == 'C') {
          mode = CreditRenderMode::Contributors;
        }
        else if (text_to_render[1] == 'S') {
          mode = CreditRenderMode::SpecialThanks;
        }
      }

        // render
      if (y >= 0 && y < screen.bounds.h) { // clip to screen
        if (mode == CreditRenderMode::Contributors && contributors[contrib_index]) {
          screen.text(contributors[contrib_index], launcher_font, Point(screen_width / 2, y), true, TextAlign::center_h);
        }
        else if (mode == CreditRenderMode::SpecialThanks && special_thanks[special_index]) {
          screen.pen = rainbow_colours[colour_index];
          colour_index++;
          if (colour_index >= number_of_colours) {
            colour_index = 0;
          }
          screen.text(special_thanks[special_index], launcher_outline_font, Point(screen_width / 2, y), true, TextAlign::center_h);
        }
        else { // render regular text
          screen.text(text_to_render, launcher_font, Point(screen_width / 2, y), true, TextAlign::center_h);
        }
      }

      // Step to next text
      if (mode == CreditRenderMode::Contributors) {
        contrib_index++;
        if (contributors[contrib_index] == nullptr) {
          mode = CreditRenderMode::Credits;
        }
      }
      if (mode == CreditRenderMode::SpecialThanks) {
        special_index++;
        if (special_thanks[special_index] == nullptr) {
          mode = CreditRenderMode::Credits;
        }
      }
      if (mode == CreditRenderMode::Credits) {
        next_index++;
      }
      y += ROW_HEIGHT;
    }

    screen.pen = bar_colour;
    screen.rectangle(Rect(0, 0, display_rect.w, cinematic_bar_height));
    screen.rectangle(Rect(0, display_rect.h - cinematic_bar_height, display_rect.w, cinematic_bar_height));

    if (y < 0) {
      reset_scrolling();
    }
  }

  void reset_scrolling() {
    start_y = screen.bounds.h - cinematic_bar_height;
    last_start_time = 0;
  }

  void update(uint32_t time) {
    if (last_start_time == 0) {
      last_start_time = time;
    }

    // calculate starting_y using the speed constant and elapsed time since start time
    start_y = screen.bounds.h - cinematic_bar_height - ((time - last_start_time) / SPEED);
  }
}

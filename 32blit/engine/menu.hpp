#pragma once

#include <string>

#include "engine/api.hpp"
#include "engine/engine.hpp"
#include "graphics/surface.hpp"

namespace blit {
  class Menu {
  public:
    struct Item {
      uint16_t id;
      const char *label;
    };

    Menu(std::string_view title, const Item *items = nullptr, int num_items = 0) : title(title), items(items), num_items(num_items),
      display_rect(0, 0, 0, 0) {
    }
    virtual ~Menu() {}

    void render() {
      screen.pen = background_colour;
      screen.rectangle(display_rect);

      // header
      screen.pen = foreground_colour;
      screen.text(title, minimal_font, Point(display_rect.x + margin_x, display_rect.y + margin_y));

      screen.h_span(Point(display_rect.x, display_rect.y + header_h), display_rect.w);

      int y = display_rect.y + header_h + margin_y;

      // selected item
      screen.pen = selected_item_background;
      screen.rectangle(Rect(display_rect.x, y + current_item * (item_h + item_spacing), display_rect.w, item_h));

      // items
      for(int i = 0; i < num_items; i++) {
        auto &item = items[i];

        render_item(item, y);

        y += item_h + item_spacing;
      }

      // footer
      screen.pen = foreground_colour;
      screen.h_span(Point(display_rect.x, display_rect.y + display_rect.h - footer_h), display_rect.w);
    }

    void update() {
      // default size
      if(display_rect.w == 0 && display_rect.h == 0) {
        display_rect.w = screen.bounds.w;
        display_rect.h = screen.bounds.h;
      }

      if(buttons.pressed & Button::DPAD_UP)
        current_item = current_item == 0 ? num_items - 1 : current_item - 1;
      else if(buttons.pressed & Button::DPAD_DOWN)
        current_item = current_item == num_items - 1 ? 0 : current_item + 1;
      else if(buttons.pressed & Button::A)
        item_activated(items[current_item]);

      update_item(items[current_item]);
    }

    void set_items(const Item *items, int num_items) {
      this->items = items;
      this->num_items = num_items;
      current_item = 0;
    }

    void set_display_rect(Rect rect) {
      display_rect = rect;
    }

    std::string_view title;

  protected:
    virtual void render_item(const Item &item, int y) const {
      screen.pen = foreground_colour;
      screen.text(item.label, minimal_font, Point(display_rect.x + margin_x, y + item_margin_y));
    }

    virtual void update_item(const Item &item) {
    }

    virtual void item_activated(const Item &item) {
    }

    const Item *items;
    int num_items;
    int current_item = 0;

    // layout
    Rect display_rect;
    const int header_h = 15, footer_h = 15;
    const int margin_x = 5, margin_y = 5;
    const int item_h = 9;
    const int item_margin_y = 1;
    const int item_spacing = 1;

    // colours
    Pen background_colour = Pen(30,  30,  50, 200);
    Pen foreground_colour = Pen(255, 255, 255);
    Pen selected_item_background = Pen(50,  50,  70);
  };
}
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

    Menu(std::string_view title, const Item *items = nullptr, int num_items = 0) : title(title), items(items), num_items(num_items) {
    }
    virtual ~Menu() {}

    void render() {
      screen.pen = background_colour;
      screen.clear();

      // header
      screen.pen = foreground_colour;
      screen.text(title, minimal_font, Point(margin_x, margin_y));

      screen.h_span(Point(0, header_h), screen.bounds.w);

      int y = header_h + margin_y;

      // selected item
      screen.pen = selected_item_background;
      screen.rectangle(Rect(0, y + current_item * (item_h + item_spacing), screen.bounds.w, item_h));

      // items
      for(int i = 0; i < num_items; i++) {
        auto &item = items[i];

        render_item(item, y);

        y += item_h + item_spacing;
      }

      // footer
      screen.pen = foreground_colour;
      screen.h_span(Point(0, screen.bounds.h - 15), screen.bounds.w);
    }

    void update() {
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

    std::string_view title;

  protected:
    virtual void render_item(const Item &item, int y) const {
      screen.pen = foreground_colour;
      screen.text(item.label, minimal_font, Point(margin_x, y + item_margin_y));
    }

    virtual void update_item(const Item &item) {
    }

    virtual void item_activated(const Item &item) {
    }

    const Item *items;
    int num_items;
    int current_item = 0;

    // layout
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
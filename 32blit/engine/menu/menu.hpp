#include "menuItem.hpp"
#include <vector>
#include <string>

#ifndef MENU_H
#define MENU_H

struct NavigationLevel {
    std::string title;
    std::vector<MenuItem> items;
    int selection;
    int offset;
    
    NavigationLevel(std::string title, std::vector<MenuItem>items, int selection, int offset): title(title), items(items), selection(selection), offset(offset) {}
};

class Menu {

    int BANNER_HEIGHT = 15;
    int MAX_SCROLL_OFFSET = BANNER_HEIGHT + 5;

    private:

        std::string _menu_title;

        std::vector<MenuItem> _menu_items;
        std::vector<NavigationLevel> _navigation_stack;
        std::string _display_title;

        int _selected_index;
        int _offset = MAX_SCROLL_OFFSET;

        int menu_y (int index);

    public:
        Menu(std::string menuTitle,std::vector<MenuItem> items);

        void render(uint32_t time);
        void update(uint32_t time);
        int min_offset ();

        int bottom_bar_yposition ();

        void draw_top_bar (uint32_t time);
        void draw_bottom_line ();
        void check_vertical_offset ();

        // Vertical selection change
        void increment_selection ();
        void decrement_selection ();

        // Single change in horizontal axis
        void pressed_right();
        void pressed_left();

        // Directional button held on
        void held_right ();
        void held_left ();
        
        // Pressed A
        void selected ();

        // Pressed B
        void back_pressed ();

        void menu_hiding();
};

#endif

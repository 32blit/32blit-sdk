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

class Menu
{
    private:

        std::string _menuTitle;

        std::vector<MenuItem> _menuItems;
        std::vector<NavigationLevel> _navigationStack;
        std::string _displayTitle;

        int min_offset ();
        int bottom_bar_yposition ();

        void draw_top_bar (uint32_t time);
        void draw_bottom_line ();
        void check_vertical_offset ();

    public:
        Menu(std::string menuTitle,std::vector<MenuItem> items);

        // Toggle this to show and hide the menu.
        bool presented = false;

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

        void render(uint32_t time);
};

#endif

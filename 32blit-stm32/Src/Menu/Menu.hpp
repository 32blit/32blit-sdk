#include "MenuItem.hpp"
#include <vector>
#include <string>
#include "32blit.hpp"

#ifndef MENU_H
#define MENU_H

using namespace blit;

struct NavigationLevel {
    std::string title;
    std::vector<MenuItem> items;
    int selection;
    
    NavigationLevel(std::string title, std::vector<MenuItem>items, int selection): title(title), items(items), selection(selection) {}
};

class Menu
{
    private:
        std::vector<MenuItem> _menuItems;
        std::vector<NavigationLevel> _navigationStack;
        std::string _displayTitle;

        void drawTopBar (uint32_t time);

    public:
        Menu(std::vector<MenuItem> items);

        void incrementSelection ();
        void decrementSelection ();

        void pressedRight();
        void pressedLeft();

        void selected ();
        void backPressed ();

        void render(uint32_t time);
};

#endif

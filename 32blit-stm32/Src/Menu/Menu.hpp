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
    int offset;
    
    NavigationLevel(std::string title, std::vector<MenuItem>items, int selection, int offset): title(title), items(items), selection(selection), offset(offset) {}
};

class Menu
{
    private:
        std::vector<MenuItem> _menuItems;
        std::vector<NavigationLevel> _navigationStack;
        std::string _displayTitle;

        void drawTopBar (uint32_t time);
        void drawBottomLine ();
        int minOffset ();
        int bottomBarYPosition ();
        void checkVerticalOffset ();

    public:
        Menu(std::vector<MenuItem> items);

        // Vertical selection change
        void incrementSelection ();
        void decrementSelection ();

        // Single change in horizontal axis
        void pressedRight();
        void pressedLeft();

        // Directional button held on
        void heldRight ();
        void heldLeft ();
        
        // Pressed A
        void selected ();

        // Pressed B
        void backPressed ();

        void render(uint32_t time);
};

#endif

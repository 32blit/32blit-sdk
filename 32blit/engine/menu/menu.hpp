#include "menuItem.hpp"
#include <vector>
#include <string>

#ifndef MENU_H
#define MENU_H

using namespace std;

struct NavigationLevel {
    string title;
    vector<MenuItem> items;
    int selection;
    int offset;
    
    NavigationLevel(string title, vector<MenuItem>items, int selection, int offset): title(title), items(items), selection(selection), offset(offset) {}
};

class Menu
{
    private:

        string _menuTitle;

        vector<MenuItem> _menuItems;
        vector<NavigationLevel> _navigationStack;
        string _displayTitle;

        int minOffset ();
        int bottomBarYPosition ();

        void drawTopBar (uint32_t time);
        void drawBottomLine ();
        void checkVerticalOffset ();

    public:
        Menu(string menuTitle,vector<MenuItem> items);

        // Toggle this to show and hide the menu.
        bool presented = false;

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

#include <string>
#include <vector>
#include "32blit.hpp"

#ifndef MENUITEM_H
#define MENUITEM_H

struct MenuItem {

    // This is used when creating an item that has children items to drill down to
    MenuItem (std::string title, std::vector<MenuItem> children);

    // This is for rows that have a slider. 'brightness' etc
    MenuItem (std::string title, float (*slider)());

    // This is for rows that have an action. 'Shut down' etc
    MenuItem (std::string title, std::string text, void (*action)());

    // This is used when the item is just an info row. Version number etc.
    MenuItem (std::string title, std::string text);
    
    void draw (unsigned int yPos, bool selected, blit::Size rowSize);
};

#endif
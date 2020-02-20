#include <string>
#include <vector>
#include "32blit.hpp"
#include "display.hpp"

#ifndef MENUITEM_H
#define MENUITEM_H

const Pen bar_background_color = Pen(40, 40, 60);

class MenuItem {

    private:

        // This is the text on the right hand side
        std::string _text;

        // Menu items to be drilled down to
        std::vector<MenuItem> _items;

        // Function to be called when clicked on
        void (*_selectCallback)() = nullptr;

        // Funtion to be called when sliding value change
        void (*_slideCallback)(float) = nullptr;

        // This is used to get the current value of the slider
        float (*_sliderGetter)(void) = nullptr;

        // Value to change by when using the direction buttons
        float leftAdjustment;
        float rightAdjustment;

    public:

        // Text on the left hand side
        std::string title;

        // This is used when creating an item that has children items to drill down to
        MenuItem (std::string itemTitle, std::vector<MenuItem> children);

        // This is for rows that have a slider. 'brightness' etc
        MenuItem (std::string itemTitle, void (*slider)(float), float (*sliderGetter)(void), float lAdjustment, float rAdjustment);

        // This is for rows that have an action. 'Shut down' etc
        MenuItem (std::string itemTitle, std::string text, void (*action)());

        // This is used when the item is just an info row. Version number etc.
        MenuItem (std::string itemTitle, std::string text);
        
        void draw (unsigned int yPos, bool selected, blit::Size rowSize);

        void pressedRight();
        void pressedLeft();
        std::vector<MenuItem> selected();
};

#endif
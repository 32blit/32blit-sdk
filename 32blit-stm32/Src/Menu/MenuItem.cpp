#include "32blit.hpp"
#include "string.h"
#include "MenuItem.hpp"

using namespace blit;

// Text on the left hand side
std::string _title;

// This is the text on the right hand side
std::string _text;

// Menu items to be drilled down to
std::vector<MenuItem> _items;

// Function to be called when clicked on
void (*_selectCallback)() = nullptr;

// Funtion to be called when sliding value change
float (*_slideCallback)() = nullptr;

// This is used when creating an item that has children items to drill down to
MenuItem::MenuItem (std::string title, std::vector<MenuItem> children)  {
    _title = title;
    _items = children;
}

// This is for rows that have a slider. 'brightness' etc
MenuItem::MenuItem (std::string title, float (*slider)()) {
    _title = title;
    _slideCallback = slider;
    _selectCallback = nullptr;
}

// This is for rows that have an action. 'Shut down' etc
MenuItem::MenuItem (std::string title, std::string text, void (*action)()) {
    _title = title;
    _text = text;
    _selectCallback = action;
    _slideCallback = nullptr;
}

// This is used when the item is just an info row. Version number etc.
MenuItem::MenuItem (std::string title, std::string text) {
    _title = title;
    _text = text;
}

void MenuItem::draw (unsigned int yPos, bool selected, Size rowSize) {

    if (selected) {
        screen.pen = Pen(50,50,70);
        screen.rectangle(Rect(0, yPos, rowSize.w, rowSize.h));
    }

    const int nestedItemY = yPos + (rowSize.h * 0.1);

    screen.pen = Pen(255, 255, 255);
    screen.text(_title, minimal_font, Point(5, nestedItemY));

    if (!_text.empty() || _selectCallback != nullptr) {
        screen.pen = Pen(255, 255, 255);
        screen.text(_title, minimal_font, Point(rowSize.w / 2, nestedItemY));
    }

    if (_selectCallback != nullptr && _text.empty()) {
        screen.pen = Pen(255, 255, 255);
        screen.text(">", minimal_font, Point(rowSize.w - 10, nestedItemY));
    }

}

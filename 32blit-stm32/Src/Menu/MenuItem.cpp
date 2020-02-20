#include "32blit.hpp"
#include "string.h"
#include "MenuItem.hpp"

using namespace blit;

// This is used when creating an item that has children items to drill down to
MenuItem::MenuItem (std::string itemTitle, std::vector<MenuItem> children)  {
    title = itemTitle;
    _items = children;
}

// This is for rows that have a slider. 'brightness' etc
MenuItem::MenuItem (std::string itemTitle, void (*slider)(float), float (*sliderGetter)(void), float lAdjustment, float rAdjustment) {
    title = itemTitle;
    _slideCallback = slider;
    _selectCallback = nullptr;
    _sliderGetter = sliderGetter;

    leftAdjustment = lAdjustment;
    rightAdjustment = rAdjustment;
}

// This is for rows that have an action. 'Shut down' etc
MenuItem::MenuItem (std::string itemTitle, std::string text, void (*action)()) {
    title = itemTitle;
    _text = text;
    _selectCallback = action;
    _slideCallback = nullptr;
}

// This is used when the item is just an info row. Version number etc.
MenuItem::MenuItem (std::string itemTitle, std::string text) {
    title = itemTitle;
    _text = text;
}

void MenuItem::draw (unsigned int yPos, bool selected, Size rowSize) {

    if (selected) {
        screen.pen = Pen(50,50,70);
        screen.rectangle(Rect(0, yPos, rowSize.w, rowSize.h));
    }

    Font font = minimal_font;
    float yTextOffset = (rowSize.h - font.char_h) / 2.0;
    float nestedItemY = yPos + yTextOffset;

    screen.pen = Pen(255, 255, 255);
    screen.text(title, font, Point(5.0, nestedItemY));

    if (!_text.empty() || _selectCallback != nullptr) {
        screen.pen = Pen(255, 255, 255);
        screen.text(_text, font, Point(rowSize.w / 2, nestedItemY));
    }

    if (_items.size() > 0) {
        screen.pen = Pen(255, 255, 255);
        screen.text(">", font, Point(rowSize.w - 10, nestedItemY));
    }

    if (_slideCallback != nullptr) {

        float sliderY = yPos + (rowSize.h - 5.0) / 2.0;

        screen.pen = bar_background_color;
        screen.rectangle(Rect(rowSize.w / 2, sliderY, 75, 5));
        screen.pen = Pen(255, 255, 255);
        screen.rectangle(Rect(rowSize.w / 2, sliderY, _sliderGetter(), 5));
    }
}

void MenuItem::pressedRight() {
    if (_slideCallback) { _slideCallback(rightAdjustment); }
}

void MenuItem::pressedLeft() {
    if (_slideCallback) { _slideCallback(leftAdjustment); }
}

std::vector<MenuItem> MenuItem::selected() {
    if (_selectCallback) { _selectCallback(); }

    return _items;
}
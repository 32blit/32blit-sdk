#include "32blit.hpp"
#include "string.h"
#include "MenuItem.hpp"

using namespace blit;

// This is used when creating an item that has children items to drill down to
MenuItem::MenuItem (string itemTitle, vector<MenuItem> children)  {
    title = itemTitle;
    _items = children;
}

// This is for rows that have a slider. 'brightness' etc
MenuItem::MenuItem (string itemTitle, void (*slider)(float), float (*sliderGetter)(void), float lAdjustment, float rAdjustment) {
    title = itemTitle;
    _slideCallback = slider;
    _selectCallback = nullptr;
    _sliderGetter = sliderGetter;

    _leftAdjustment = lAdjustment;
    _rightAdjustment = rAdjustment;
}

// This is for rows that have an action. 'Shut down' etc
MenuItem::MenuItem (string itemTitle, string text, void (*action)()) {
    title = itemTitle;
    _text = text;
    _selectCallback = action;
    _slideCallback = nullptr;
}

// This is used when the item is just an info row. Version number etc.
MenuItem::MenuItem (string itemTitle, string text) {
    title = itemTitle;
    _text = text;
}

MenuItem::MenuItem (string itemTitle, vector<OptionItem> options, void (*optionChanged)(OptionItem)) {

    title = itemTitle;
    _optionItems = options;
    _currentOptionIndex = 0;
    _optionChanged = optionChanged;
}

struct OptionTextInfo {
    string text;
    float xPosition;

    OptionTextInfo(string text,float xPosition): text(text), xPosition(xPosition) {}
};

/*
    This funciton will hopefully provide information on how to draw the text within the  < an >
    So it should centralise text so its like this 
    <      Hard     >
    or 
    <   Difficult   >
    or if it's too long to truncate
    <This is a lon..>
*/
OptionTextInfo optionItemLocation (string text,int font_width, int minX, int maxX) {
    int availableChars = (maxX - minX) / (font_width - 1);

    debug("\n\n\n\n\n\n\n\n\n\n\n\n" + to_string(availableChars) + "\n" + to_string(font_width) + "\n" + to_string(minX) + "\n" + to_string(maxX));


    if (text.size() == availableChars) {
        // perfect fit
        return OptionTextInfo(text, minX + font_width);
    } else if (text.size() > availableChars) {
        // Not going to fit
        string substring = text.substr(0,availableChars-2);
        return OptionTextInfo(substring + "..", minX + font_width);
    } else {
        // space to spare
        int delta = (maxX - minX) - (text.size() * font_width);
        return OptionTextInfo(text, minX + delta / 2.0f);
    }
}

void MenuItem::draw (unsigned int yPos, bool selected, Size rowSize) {

    // Show as an item thats in focus. Draws a slightly highlighted colour behind the text
    if (selected) {
        screen.pen = Pen(50,50,70);
        screen.rectangle(Rect(0, yPos, rowSize.w, rowSize.h));
    }

    Font font = minimal_font;
    uint8_t yTextOffset = uint8_t((rowSize.h - font.char_h) / 2.0);
    uint8_t nestedItemY = uint8_t(yPos + yTextOffset);

    screen.pen = Pen(255, 255, 255);
    screen.text(title, font, Point(5.0, nestedItemY));

    // Text that appears on the right hand side. Press A etc
    if (!_text.empty() || _selectCallback != nullptr) {
        screen.pen = Pen(255, 255, 255);
        screen.text(_text, font, Point(rowSize.w / 2, nestedItemY));
    }

    // This is a menu item that can be drilled down into
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

    if (!_optionItems.empty()) {
        screen.pen = Pen(255, 255, 255);

        int gtLeft = rowSize.w - 10;    // >
        int ltLeft = rowSize.w / 2.0;   // <

        screen.text(">", font, Point(gtLeft, nestedItemY));
        screen.text("<", font, Point(ltLeft, nestedItemY));

        OptionItem currentItem = _optionItems.at(_currentOptionIndex);

        OptionTextInfo info = optionItemLocation(currentItem.title,font.char_w, ltLeft + font.char_w, gtLeft);
        screen.text(info.text, font, Point(info.xPosition, nestedItemY));
    }
}

void MenuItem::heldRight() {
    if (_slideCallback) { _slideCallback(_rightAdjustment); }

}

void MenuItem::heldLeft() {
    if (_slideCallback) { _slideCallback(_leftAdjustment); }
}

void MenuItem::pressedLeft () {
    // if the item has options. Difficulty selection etc
    if (!_optionItems.empty() && --_currentOptionIndex < 0) {
        _currentOptionIndex = int(_optionItems.size()) - 1;
    }
}

void MenuItem::pressedRight () {
    // if the item has options. Difficulty selection etc
    if (!_optionItems.empty()) {
        _currentOptionIndex = ++_currentOptionIndex % int(_optionItems.size());
    }
}

vector<MenuItem> MenuItem::selected() {
    if (_selectCallback) { _selectCallback(); }

    return _items;
}
#include "32blit.hpp"
#include "string.h"
#include "menuItem.hpp"
#include "../../graphics/surface.hpp"

using namespace blit;
using namespace std;

const blit::Pen bar_background_color = blit::Pen(40, 40, 60);

// This is used when creating an item that has children items to drill down to
MenuItem::MenuItem (string itemTitle, vector<MenuItem> children)  {
    _title = itemTitle;
    _items = children;
}

// This is for rows that have a slider. 'brightness' etc
MenuItem::MenuItem (string itemTitle, void (*slider)(float), float (*sliderGetter)(void), float lAdjustment, float rAdjustment) {
    _title = itemTitle;
    _slide_callback = slider;
    _select_callback = nullptr;
    _slider_getter = sliderGetter;

    _left_adjustment = lAdjustment;
    _right_adjustment = rAdjustment;
}

// This is for rows that have an action. 'Shut down' etc
MenuItem::MenuItem (std::string itemTitle, std::string text, std::string confirmText, void (*action)()) {
    _title = itemTitle;
    _confirm_text = confirmText;
    _text = text;
    _select_callback = action;
    _slide_callback = nullptr;
}

// This is for rows that have an action. 'Shut down' etc
MenuItem::MenuItem (string itemTitle, string text, void (*action)()) {
    _title = itemTitle;
    _text = text;
    _select_callback = action;
    _slide_callback = nullptr;
}

// This is used when the item is just an info row. Version number etc.
MenuItem::MenuItem (string itemTitle, string text) {
    _title = itemTitle;
    _text = text;
}

MenuItem::MenuItem (string itemTitle, vector<OptionItem> options, void (*optionChanged)(OptionItem)) {

    _title = itemTitle;
    _option_items = options;
    _current_option_index = 0;
    _option_changed = optionChanged;
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
OptionTextInfo option_item_location (string text,int font_width, int minX, int maxX) {
    unsigned int availableChars = (maxX - minX) / (font_width - 1); // with the font width being right-ish, subtracting 1 seems about right on average

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

void MenuItem::draw (unsigned int yPos, bool selected, int row_width, int row_height) {

    // Show as an item thats in focus. Draws a slightly highlighted colour behind the text
    if (selected) {
        screen.pen = Pen(50,50,70);
        screen.rectangle(Rect(0, yPos, row_width, row_height));
    }

    Font font = minimal_font;
    uint8_t y_text_offset = uint8_t((row_height - font.char_h) / 2.0);
    uint8_t nested_item_y = uint8_t(yPos + y_text_offset);

    screen.pen = Pen(255, 255, 255);
    screen.text(display_text(), font, Point(5.0, nested_item_y));

    // Text that appears on the right hand side. Press A etc
    if (!_text.empty() || _select_callback != nullptr) {
        screen.pen = Pen(255, 255, 255);
        screen.text(_text, font, Point(row_width / 2, nested_item_y));
    }

    // This is a menu item that can be drilled down into
    if (_items.size() > 0) {
        screen.pen = Pen(255, 255, 255);
        screen.text(">", font, Point(row_width - 10, nested_item_y));
    }

    if (_slide_callback != nullptr) {

        float sliderY = yPos + (row_height - 5.0) / 2.0;

        screen.pen = bar_background_color;
        screen.rectangle(Rect(row_width / 2, sliderY, 75, 5));
        screen.pen = Pen(255, 255, 255);
        screen.rectangle(Rect(row_width / 2, sliderY, _slider_getter(), 5));
    }

    if (!_option_items.empty()) {
        screen.pen = Pen(255, 255, 255);

        int gtLeft = row_width - 10;
        int ltLeft = row_width / 2.0;

        screen.text(">", font, Point(gtLeft, nested_item_y));
        screen.text("<", font, Point(ltLeft, nested_item_y));

        OptionItem currentItem = _option_items.at(_current_option_index);

        OptionTextInfo info = option_item_location(currentItem.title,font.char_w, ltLeft + font.char_w, gtLeft);
        screen.text(info.text, font, Point(info.xPosition, nested_item_y));
    }
}

void MenuItem::held_right() {
    if (_slide_callback) { _slide_callback(_right_adjustment); }
}

void MenuItem::held_left() {
    if (_slide_callback) { _slide_callback(_left_adjustment); }
}

void MenuItem::pressed_left () {

    if (_slide_callback) { 
        _slide_callback(_right_adjustment); 
    } else if (!_option_items.empty() && --_current_option_index < 0) {
    // if the item has options. Difficulty selection etc
        _current_option_index = int(_option_items.size()) - 1;
    }
}

void MenuItem::pressed_right () {

    if (_slide_callback) { 
        _slide_callback(_right_adjustment); 
    } 
    else if (!_option_items.empty() && ++_current_option_index == int(_option_items.size())) {
    // if the item has options. Difficulty selection etc
        _current_option_index = 0;
    }
}

vector<MenuItem> MenuItem::selected() {

    if ((!has_confirm_text() && _select_callback) ||                                 // no confirmation required
        (has_confirm_text() && _is_displaying_confirmation && _select_callback)) {   // confirmation appeared

        // Either we dont need confirmation, or it's already displayed
        _select_callback();
    } else if (has_confirm_text() && !_is_displaying_confirmation) {

        // Menu item requires confirmation, so change the flag to display it. 
        _is_displaying_confirmation = true;
    }

    return _items;
}

void MenuItem::reset_display_text() {
    _is_displaying_confirmation = false;
}

std::string MenuItem::display_text() {
    return _is_displaying_confirmation ? _confirm_text : _title;
}

bool MenuItem::has_confirm_text() {
    return !_confirm_text.empty();
}
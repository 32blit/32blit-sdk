#include "menu.hpp"
#include "menuItem.hpp"
#include "../input.hpp"
#include "../engine.hpp"

using namespace blit;
using namespace std;

Size screen_size ();

Pen bar_background_color = Pen(40, 40, 60);
Pen bannerColour = Pen(50,50,50,150);

int16_t _row_height = 12;

int Menu::menu_y (int index) { return index * _row_height + _offset; }

Menu::Menu(string menuTitle, vector<MenuItem> items):_menu_title(menuTitle), _menu_items(items) {}

signed int Menu::min_offset () {

    int16_t rows_available_on_screen = (screen_size().h - BANNER_HEIGHT) / _row_height;
    int16_t items_on_screen = min(int16_t(_menu_items.size()), rows_available_on_screen);

    if (items_on_screen < rows_available_on_screen) { return MAX_SCROLL_OFFSET; }

    return -(int16_t(_menu_items.size()) * _row_height) + (items_on_screen * _row_height);
}

int Menu::bottom_bar_yposition () {
    return screen_size().h - BANNER_HEIGHT;
}

void Menu::check_vertical_offset () {

    int screen_height = screen_size().h;
    int select_y_pos = menu_y(_selected_index);

    if (select_y_pos >= screen_height - (_row_height * 3)){
        _offset = max(_offset -_row_height, min_offset());
    } else if (select_y_pos <= _row_height * 2) {
        _offset = min(MAX_SCROLL_OFFSET, _offset + _row_height);
    }
}

void Menu::increment_selection () { 
    if (_selected_index == _menu_items.size() - 1) {
        // send me back to the top, thanks
        _selected_index = 0;
        _offset = MAX_SCROLL_OFFSET;
    } else {
        _selected_index++;
        check_vertical_offset();
    }
}

void Menu::decrement_selection () {
    if (_selected_index == 0) {
        // go all the way to the end
        _selected_index = _menu_items.size() - 1;
        _offset = min_offset();
    } else {
        _selected_index--;
        check_vertical_offset();
    }
}

void Menu::pressed_right() { _menu_items[_selected_index].pressed_right(); }
void Menu::pressed_left() { _menu_items[_selected_index].pressed_left(); }

void Menu::held_right() { _menu_items[_selected_index].held_right(); }
void Menu::held_left() { _menu_items[_selected_index].held_left(); }

void Menu::selected() { 
    auto childItems = _menu_items[_selected_index].selected(); 

    if (!childItems.empty()){

        // stash away the current menu
        NavigationLevel level = NavigationLevel(
            _display_title.empty() ? _display_title : _menu_title,
            _menu_items,
            _selected_index,
            _offset
            );

        _display_title = _menu_items[_selected_index].display_text();

        _selected_index = 0;
        _menu_items = childItems;
        _navigation_stack.push_back(level);
        _offset = MAX_SCROLL_OFFSET;
    }
}

void Menu::back_pressed() {

    if(!_navigation_stack.empty()) {
        auto back = _navigation_stack.back();

        // unravel previous menu
        _menu_items = back.items;
        _selected_index = back.selection;
        _display_title = back.title.size() > 0 ? back.title : _menu_title;
        _offset = back.offset;
        _navigation_stack.pop_back();
    }
}

Size screen_size () {
    return blit::screen.bounds;
}

void Menu::draw_top_bar (uint32_t time) {

    screen.pen = bannerColour;
    screen.rectangle(Rect(0,0,screen_size().w,BANNER_HEIGHT));

    screen.pen = Pen(255, 255, 255);

    screen.text(_display_title.empty() ? _menu_title : _display_title , minimal_font, Point(5, 5));
    screen.text("bat", minimal_font, Point(screen_size().w / 2, 5));
    uint16_t battery_meter_width = 55;
    battery_meter_width = float(battery_meter_width) * (blit::battery - 3.0f) / 1.1f;
    battery_meter_width = max((uint16_t)0, min((uint16_t)55, battery_meter_width));

    screen.pen = bar_background_color;
    screen.rectangle(Rect((screen_size().w / 2) + 20, 6, 55, 5));

    switch(battery_vbus_status){
        case 0b00: // Unknown
            screen.pen = Pen(255, 128, 0);
            break;
        case 0b01: // USB Host
            screen.pen = Pen(0, 255, 0);
            break;
        case 0b10: // Adapter Port
            screen.pen = Pen(0, 255, 0);
            break;
        case 0b11: // OTG
            screen.pen = Pen(255, 0, 0);
            break;
    }

    screen.rectangle(Rect((screen_size().w / 2) + 20, 6, battery_meter_width, 5));
    if (battery_charge_status == 0b01 || battery_charge_status == 0b10){
        uint16_t battery_fill_width = uint32_t(time / 100.0f) % battery_meter_width;
        battery_fill_width = max((uint16_t)0, min((uint16_t)battery_meter_width, battery_fill_width));
        screen.pen = Pen(100, 255, 200);
        screen.rectangle(Rect((screen_size().w / 2) + 20, 6, battery_fill_width, 5));
    }

    // Horizontal Line
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, BANNER_HEIGHT, screen_size().w, 1));
}

void Menu::draw_bottom_line () {

    screen.pen = bannerColour;
    screen.rectangle(Rect(0,bottom_bar_yposition(),screen_size().w,BANNER_HEIGHT));

    // Bottom horizontal Line
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, bottom_bar_yposition(), screen_size().w, 1));
}

void Menu::render(uint32_t time) {

    screen.pen = Pen(30, 30, 50, 200);
    screen.clear();

    for (int i = 0; i < int(_menu_items.size()); i ++) {
        MenuItem item = _menu_items[i];
        int yPosition = menu_y(i);

        // might be worth adding in check to see if they're on screen to save on text rendering?
        item.draw(yPosition, _selected_index == i, screen_size().w, _row_height);
    }

    draw_top_bar(time);
    draw_bottom_line();
    
}

void Menu::menu_hiding () {

    for (auto &item : _menu_items) {
        item.reset_display_text();
    }
}
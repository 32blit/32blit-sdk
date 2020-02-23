#include "menu.hpp"
#include "menuItem.hpp"
#include "../input.hpp"
#include "../engine.hpp"

using namespace blit;

Size screenSize ();

int BANNER_HEIGHT = 15;
int MAX_SCROLL_OFFSET = BANNER_HEIGHT + 5;

Pen bar_background_color = Pen(40, 40, 60);
Pen bannerColour = Pen(50,50,50,150);

int _selectedIndex;
int _rowHeight = 12;
int offset = MAX_SCROLL_OFFSET;

int menu_y (int index) { return index * _rowHeight + offset; }

Menu::Menu(string menuTitle, vector<MenuItem> items):_menuTitle(menuTitle), _menuItems(items) {}

int Menu::minOffset () {

    int rowsAvailableOnscreen = (screenSize().h - BANNER_HEIGHT) / _rowHeight;
    int itemsOnScreen = min(int(_menuItems.size()), rowsAvailableOnscreen);

    if (itemsOnScreen < rowsAvailableOnscreen) { return MAX_SCROLL_OFFSET; }

    return -(_menuItems.size() * _rowHeight) + (itemsOnScreen * _rowHeight);
}

int Menu::bottomBarYPosition () {
    return screenSize().h - BANNER_HEIGHT;
}

void Menu::checkVerticalOffset () {

    int screenHeight = screenSize().h;
    int selectYPos = menu_y(_selectedIndex);

    if (selectYPos >= screenHeight - (_rowHeight * 3)){
        offset = max(offset -_rowHeight, minOffset());
    } else if (selectYPos <= _rowHeight * 2) {
        offset = min(MAX_SCROLL_OFFSET, offset + _rowHeight);
    }
}

void Menu::incrementSelection () { 
    if (_selectedIndex == _menuItems.size() - 1) {
        // send me back to the top, thanks
        _selectedIndex = 0;
        offset = MAX_SCROLL_OFFSET;
    } else {
        _selectedIndex++;
        checkVerticalOffset();
    }
}

void Menu::decrementSelection () {
    if (_selectedIndex == 0) {
        // go all the way to the end
        _selectedIndex = _menuItems.size() - 1;
        offset = minOffset();
    } else {
        _selectedIndex--;
        checkVerticalOffset();
    }
}

void Menu::pressedRight() { _menuItems[_selectedIndex].pressedRight(); }
void Menu::pressedLeft() { _menuItems[_selectedIndex].pressedLeft(); }

void Menu::heldRight() { _menuItems[_selectedIndex].heldRight(); }
void Menu::heldLeft() { _menuItems[_selectedIndex].heldLeft(); }

void Menu::selected() { 
    auto childItems = _menuItems[_selectedIndex].selected(); 

    if (!childItems.empty()){

        // stash away the current menu
        NavigationLevel level = NavigationLevel(
            _displayTitle.empty() ? _displayTitle : _menuTitle,
            _menuItems,
            _selectedIndex,
            offset
            );

        _displayTitle = _menuItems[_selectedIndex].title;

        _selectedIndex = 0;
        _menuItems = childItems;
        _navigationStack.push_back(level);
        offset = MAX_SCROLL_OFFSET;
    }
}

void Menu::backPressed() {

    if(!_navigationStack.empty()) {
        auto back = _navigationStack.back();

        // unravel previous menu
        _menuItems = back.items;
        _selectedIndex = back.selection;
        _displayTitle = back.title.size() > 0 ? back.title : _menuTitle;
        offset = back.offset;
        _navigationStack.pop_back();
    }
}

Size screenSize () {
    return blit::screen.bounds;
}

void Menu::drawTopBar (uint32_t time) {

    screen.pen = bannerColour;
    screen.rectangle(Rect(0,0,screenSize().w,BANNER_HEIGHT));

    screen.pen = Pen(255, 255, 255);

    screen.text(_displayTitle.empty() ? _menuTitle : _displayTitle , minimal_font, Point(5, 5));
    screen.text("bat", minimal_font, Point(screenSize().w / 2, 5));
    uint16_t battery_meter_width = 55;
    battery_meter_width = float(battery_meter_width) * (blit::battery - 3.0f) / 1.1f;
    battery_meter_width = max((uint16_t)0, min((uint16_t)55, battery_meter_width));

    screen.pen = bar_background_color;
    screen.rectangle(Rect((screenSize().w / 2) + 20, 6, 55, 5));

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

    screen.rectangle(Rect((screenSize().w / 2) + 20, 6, battery_meter_width, 5));
    if (battery_charge_status == 0b01 || battery_charge_status == 0b10){
        uint16_t battery_fill_width = uint32_t(time / 100.0f) % battery_meter_width;
        battery_fill_width = max((uint16_t)0, min((uint16_t)battery_meter_width, battery_fill_width));
        screen.pen = Pen(100, 255, 200);
        screen.rectangle(Rect((screenSize().w / 2) + 20, 6, battery_fill_width, 5));
    }

    // Horizontal Line
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, BANNER_HEIGHT, screenSize().w, 1));
}

void Menu::drawBottomLine () {

    screen.pen = bannerColour;
    screen.rectangle(Rect(0,bottomBarYPosition(),screenSize().w,BANNER_HEIGHT));

    // Bottom horizontal Line
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, bottomBarYPosition(), screenSize().w, 1));
}

void Menu::render(uint32_t time) {

    if (!presented) { return; }

    screen.pen = Pen(30, 30, 50, 200);
    screen.clear();

    for (int i = 0; i < int(_menuItems.size()); i ++) {
        MenuItem item = _menuItems[i];
        int yPosition = menu_y(i);

        // might be worth adding in check to see if they're on screen to save on text rendering?
        item.draw(yPosition, _selectedIndex == i, screenSize().w, _rowHeight);
    }

    drawTopBar(time);
    drawBottomLine();
}

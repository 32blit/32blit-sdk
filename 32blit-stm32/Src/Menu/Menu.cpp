#include "Menu.hpp"
#include "MenuItem.hpp"
#include "32blit.hpp"
#include "display.hpp"

using namespace blit;

std::string SYSTEM_TITLE = "System Menu";

int _selectedIndex;
int _rowHeight = 12;

int offset = 20;
int menu_y (int index) { return index * _rowHeight + offset; }

Menu::Menu(std::vector<MenuItem> items): _menuItems(items) {}

void Menu::incrementSelection () { _selectedIndex = ++_selectedIndex % int(_menuItems.size()); }

void Menu::decrementSelection () {
    if (_selectedIndex == 0) {
        _selectedIndex = _menuItems.size() - 1;
    } else {
        _selectedIndex--;
    }
}

void Menu::pressedRight() { _menuItems[_selectedIndex].pressedRight(); }
void Menu::pressedLeft() { _menuItems[_selectedIndex].pressedLeft(); }

void Menu::selected() { 
    auto childItems = _menuItems[_selectedIndex].selected(); 

    if (!childItems.empty()){

        // stash away the current menu
        NavigationLevel level = NavigationLevel(
            _displayTitle.empty() ? _displayTitle : SYSTEM_TITLE,
            _menuItems,
            _selectedIndex
            );

        _displayTitle = _menuItems[_selectedIndex].title;

        _selectedIndex = 0;
        _menuItems = childItems;
        _navigationStack.push_back(level);
        
    }
}

void Menu::backPressed() {

    if(!_navigationStack.empty()) {
        auto back = _navigationStack.back();

        _menuItems = back.items;
        _selectedIndex = back.selection;
        _displayTitle = back.title.size() > 0 ? back.title : SYSTEM_TITLE;

        _navigationStack.pop_back();
    }
}

Size screenSize () {
    int screen_width = 160;
    int screen_height = 120;
    
    if (display::mode == blit::ScreenMode::hires) {
        screen_width = 320;
        screen_height = 240;
    }

    return Size(screen_width,screen_height);
}

void Menu::drawTopBar (uint32_t time) {
    screen.pen = Pen(255, 255, 255);

    screen.text(_displayTitle.empty() ? SYSTEM_TITLE : _displayTitle , minimal_font, Point(5, 5));
    screen.text("bat", minimal_font, Point(screenSize().w / 2, 5));
    uint16_t battery_meter_width = 55;
    battery_meter_width = float(battery_meter_width) * (blit::battery - 3.0f) / 1.1f;
    battery_meter_width = std::max((uint16_t)0, std::min((uint16_t)55, battery_meter_width));

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
        battery_fill_width = std::max((uint16_t)0, std::min((uint16_t)battery_meter_width, battery_fill_width));
        screen.pen = Pen(100, 255, 200);
        screen.rectangle(Rect((screenSize().w / 2) + 20, 6, battery_fill_width, 5));
    }

    // Horizontal Line
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, 15, screenSize().w, 1));
}

void drawBottomLine () {
    // Bottom horizontal Line
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, screenSize().h - 15, screenSize().w, 1));
}

void Menu::render(uint32_t time) {

    screen.pen = Pen(30, 30, 50, 200);
    screen.clear();

    drawTopBar(time);
    drawBottomLine();

    for (int i = 0; i < int(_menuItems.size()); i ++) {
        MenuItem item = _menuItems[i];
        item.draw(menu_y(i), _selectedIndex == i, Size(screenSize().w, _rowHeight));
    }
}

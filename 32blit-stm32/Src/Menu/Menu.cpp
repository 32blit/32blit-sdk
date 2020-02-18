#include "Menu.hpp"
#include "MenuItem.hpp"
#include "32blit.hpp"

std::vector<MenuItem> _menuItems;
int _selectedIndex;
Size _rowSize;
Size _screenSize;

int offset = 20;
int menu_y (int index) { return index * _rowSize.h + offset; }
const Pen bar_background_color = Pen(40, 40, 60);

Menu::Menu(std::vector<MenuItem> items, Size rowSize, Size screenSize) { 
    _menuItems = items;
    _screenSize = screenSize;
    _rowSize = rowSize;
    _selectedIndex = rowSize.w;
}

void Menu::incrementSelection () { _selectedIndex++; } //_selectedIndex = _selectedIndex++ % int(_menuItems.size()); }

void Menu::decrementSelection () {
    if (_selectedIndex == 0) {
        _selectedIndex = _menuItems.size() - 1;
    } else {
        _selectedIndex--;
    }
}

void drawTopBar (uint32_t time) {
    screen.pen = Pen(255, 255, 255);

    screen.text("System Menu", minimal_font, Point(5, 5));
    screen.text("bat", minimal_font, Point(_rowSize.w / 2, 5));
    uint16_t battery_meter_width = 55;
    battery_meter_width = float(battery_meter_width) * (blit::battery - 3.0f) / 1.1f;
    battery_meter_width = std::max((uint16_t)0, std::min((uint16_t)55, battery_meter_width));

    screen.pen = bar_background_color;
    screen.rectangle(Rect((_rowSize.w / 2) + 20, 6, 55, 5));

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
    screen.rectangle(Rect((_screenSize.w / 2) + 20, 6, battery_meter_width, 5));
    if(battery_charge_status == 0b01 || battery_charge_status == 0b10){
        uint16_t battery_fill_width = uint32_t(time / 100.0f) % battery_meter_width;
        battery_fill_width = std::max((uint16_t)0, std::min((uint16_t)battery_meter_width, battery_fill_width));
        screen.pen = Pen(100, 255, 200);
        screen.rectangle(Rect((_screenSize.w / 2) + 20, 6, battery_fill_width, 5));
    }

    // Horizontal Line
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, 15, _rowSize.w, 1));
}

void drawBottomLine () {
    // Bottom horizontal Line
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, _screenSize.h - 15, _screenSize.w, 1));
}

void Menu::render(uint32_t time) {

    screen.pen = Pen(30, 30, 50, 200);
    screen.clear();

    drawTopBar(time);
    drawBottomLine();

    for (int i = 0; i < int(_menuItems.size()); i ++) {
        MenuItem item = _menuItems[i];
        item.draw(menu_y(i), _selectedIndex == i, _rowSize);
    }
    
    screen.text(std::to_string(_rowSize.w) + "\n" + std::to_string(_rowSize.h) + "\n" + std::to_string(_selectedIndex),minimal_font,Point(50,50));
}

#import "MenuItem.hpp"
// float menu_selection_y (MenuItem item) { return menu_y(item) - 1; }
// Point menu_title_origin (MenuItem item) { return Point(5, item * 10 + 20); }
// Point press_a_origin (MenuItem item, float screen_width) { return Point(screen_width/2, item * 10 + 20); }
// Rect menu_item_frame (MenuItem item, float screen_width) { return Rect (0, item * 10 + 19, screen_width, 9); }

using namespace blit;

struct Menu {

    std::vector<MenuItem> menuItems;
    int selectedIndex = 0;
    Size rowSize;
    Size screenSize;

    int offset = 20;
    int menu_y (int index) { return index * rowSize.h + offset; }
    const Pen bar_background_color = Pen(40, 40, 60);
    
    Menu(std::vector<MenuItem> &items, Size rowSize, Size screenSize): menuItems(items), rowSize(rowSize), screenSize(screenSize) {}

    void incrementSelection () { selectedIndex = ++selectedIndex % menuItems.size(); }

    void decrementSelection () {
        if (selectedIndex == 0) {
            selectedIndex = menuItems.size() - 1;
        } else {
            selectedIndex--;
        }
    }

    void drawTopBar (uint32_t time) {
        screen.pen = Pen(255, 255, 255);

        screen.text("System Menu", minimal_font, Point(5, 5));
        screen.text("bat", minimal_font, Point(rowSize.w / 2, 5));
        uint16_t battery_meter_width = 55;
        battery_meter_width = float(battery_meter_width) * (blit::battery - 3.0f) / 1.1f;
        battery_meter_width = std::max((uint16_t)0, std::min((uint16_t)55, battery_meter_width));

        screen.pen = bar_background_color;
        screen.rectangle(Rect((rowSize.w / 2) + 20, 6, 55, 5));

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
        screen.rectangle(Rect((screenSize.w / 2) + 20, 6, battery_meter_width, 5));
        if(battery_charge_status == 0b01 || battery_charge_status == 0b10){
            uint16_t battery_fill_width = uint32_t(time / 100.0f) % battery_meter_width;
            battery_fill_width = std::max((uint16_t)0, std::min((uint16_t)battery_meter_width, battery_fill_width));
            screen.pen = Pen(100, 255, 200);
            screen.rectangle(Rect((screenSize.w / 2) + 20, 6, battery_fill_width, 5));
        }

        // Horizontal Line
        screen.pen = Pen(255, 255, 255);
        screen.rectangle(Rect(0, 15, rowSize.w, 1));
    }

    void drawBottomLine () {
        // Bottom horizontal Line
        screen.pen = Pen(255, 255, 255);
        screen.rectangle(Rect(0, screenSize.h - 15, screenSize.w, 1));
    }

    void render(uint32_t time) {

        screen.pen = Pen(30, 30, 50, 200);
        screen.clear();

        drawTopBar(time);

        for (unsigned int i = 0; i < menuItems.size(); i ++) {
            const MenuItem item = menuItems[i];
            item.draw(menu_y(i), selectedIndex == i, rowSize);
        }
    }

};
#include "32blit.hpp"
#include "string.h"

using namespace blit;

struct MenuItem {
    
    // Text on the left hand side
    std::string title;

    // This is the text on the right hand side
    std::string text;
    
    // Menu items to be drilled down to
    std::vector<MenuItem> childItems;
    
    // Function to be called when clicked on
    void (*selectCallback)() = nullptr;

    // Funtion to be called when sliding value change
    float (*slideCallback)() = nullptr;

    // This is used when creating an item that has children items to drill down to
    MenuItem (std::string title, std::vector<MenuItem> children): title(title), childItems(children) {}

    // This is for rows that have a slider. 'brightness' etc
    MenuItem (std::string title, float (*slider)()): title(title), slideCallback(slider) {}

    // This is for rows that have an action. 'Shut down' etc
    MenuItem (std::string title = "", std::string text = "Press A", void (*action)()=nullptr): title(title), text(text), selectCallback(action) {}

    // This is used when the item is just an info row. Version number etc.
    MenuItem (std::string title, std::string text): title(title), text(text) {}
    
    void draw (unsigned int yPos, bool selected, Size rowSize) {

        if (selected) {
            screen.pen = Pen(50,50,70);
            screen.rectangle(Rect(0, yPos, rowSize.w, rowSize.h));
        }

        const int nestedItemY = yPos + (rowSize.h * 0.1);

        screen.pen = Pen(255, 255, 255);
        screen.text(title, minimal_font, Point(5, nestedItemY));

        if (!text.empty() || selectCallback != nullptr) {
            screen.pen = Pen(255, 255, 255);
            screen.text(title, minimal_font, Point(rowSize.w / 2, nestedItemY));
        }

        if (selectCallback != nullptr && text.empty()) {
            screen.pen = Pen(255, 255, 255);
            screen.text(">", minimal_font, Point(rowSize.w - 10, nestedItemY));
        }

    }

};
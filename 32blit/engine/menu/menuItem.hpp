#include <string>
#include <vector>

#ifndef MENUITEM_H
#define MENUITEM_H

using namespace std;

struct OptionItem {

    // Title of the selection
    string title;

    // Given value. This is what identifies the selection. Should be unique.
    int value;

    OptionItem(string title, int value): title(title), value(value) {}
};

class MenuItem {

    private:

        // This is the text on the right hand side
        string _text;

        // Menu items to be drilled down to
        vector<MenuItem> _items;

        // Options that can be toggled through
        vector<OptionItem> _optionItems;

        // Item that is displayed/ selected
        int _currentOptionIndex;

        // Function to be called when clicked on
        void (*_selectCallback)() = nullptr;

        // Funtion to be called when sliding value change
        void (*_slideCallback)(float) = nullptr;

        // This is used to get the current value of the slider
        float (*_sliderGetter)(void) = nullptr;

        // Callback to be called when option item is changed
        void (*_optionChanged)(OptionItem);

        // Value to change by when using the direction buttons
        float _leftAdjustment;
        float _rightAdjustment;

    public:

        // Text on the left hand side
        string title;

        // This is used when creating an item that has children items to drill down to
        MenuItem (string itemTitle, vector<MenuItem> children);

        // This is for rows that have a slider. 'brightness' etc
        MenuItem (string itemTitle, void (*slider)(float), float (*sliderGetter)(void), float lAdjustment, float rAdjustment);

        // This is for rows that have an action. 'Shut down' etc
        MenuItem (string itemTitle, string text, void (*action)());

        // This is used when the item is just an info row. Version number etc.
        MenuItem (string itemTitle, string text);

        // This is used for selection - difficulty, for example.
        MenuItem (string itemTitle, vector<OptionItem> options, void (*optionChanged)(OptionItem));
        
        void draw (unsigned int yPos, bool selected, int rowWidth, int rowHeight);

        // We're interested in scrolling through these
        void heldRight();
        void heldLeft();

        // We're not interested in scrolling through these at breakneck speed
        void pressedRight();
        void pressedLeft();

        vector<MenuItem> selected();
};

#endif
#include "MenusDataSource.hpp"
#include <vector>

std::vector<MenuItem> items;

std::vector<MenuItem> MenusDataSource::menuItems() {
    return items;
};

void createMenuItems () {

    items.push_back(
        MenuItem("Vibrate", "Press A")
    );

}

MenusDataSource::MenusDataSource () { createMenuItems (); }  
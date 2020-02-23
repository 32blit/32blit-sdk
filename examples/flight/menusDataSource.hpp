#include "32blit.hpp"
#include "engine/menu/menuItem.hpp"

#ifndef MENUDATASOURCE_H
#define MENUDATASOURCE_H

struct MenusDataSource {

    std::vector<MenuItem> menuItems();
    MenusDataSource (); 
};

#endif
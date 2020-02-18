#include "MenuItem.hpp"



#ifndef MENUDATASOURCE_H
#define MENUDATASOURCE_H

struct MenusDataSource {

    // __attribute__((section(".persist"))) Persist persist;

    std::vector<MenuItem> menuItems();

    MenusDataSource (); 
};

#endif
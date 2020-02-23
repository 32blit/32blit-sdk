#include "32blit.hpp"
#include "engine/menu/menuItem.hpp"

#ifndef MENUDATASOURCE_H
#define MENUDATASOURCE_H

class FirmwareMenusDataSource {

    private:

        std::vector<MenuItem> _items;
        void createSystemMenuItems ();

    public:

        std::vector<MenuItem> menuItems();
        FirmwareMenusDataSource (); 
};

#endif
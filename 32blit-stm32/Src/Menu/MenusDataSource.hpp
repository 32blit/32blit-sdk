#include "MenuItem.hpp"



#ifndef MENUDATASOURCE_H
#define MENUDATASOURCE_H

class MenusDataSource {

    private:


    public:

    std::vector<MenuItem> menuItems();
    void(*switch_execution)();

    MenusDataSource (); 
};

#endif
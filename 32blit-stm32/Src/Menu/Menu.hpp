#include "MenuItem.hpp"
#include <vector>
#include "32blit.hpp"

#ifndef MENU_H
#define MENU_H

using namespace blit;

class Menu
{
    private:
        std::vector<MenuItem> _menuItems;


    public:
        Menu(std::vector<MenuItem> items);

        void incrementSelection ();
        void decrementSelection ();

        void pressedRight();
        void pressedLeft();

        void render(uint32_t time);
};

#endif

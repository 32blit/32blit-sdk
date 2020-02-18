#include "MenuItem.hpp"
#include <vector>
#include "32blit.hpp"

#ifndef MENU_H
#define MENU_H

using namespace blit;

struct Menu
{
    Menu(std::vector<MenuItem> items, Size rowSize, Size screenSize);

    void incrementSelection ();
    void decrementSelection ();
    void render(uint32_t time);
};

#endif
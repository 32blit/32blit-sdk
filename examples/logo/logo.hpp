#pragma once

#include <cstdint>

#include "32blit.hpp"

//extern surface fb;

void init();
void update(uint32_t time);
void render(uint32_t time);
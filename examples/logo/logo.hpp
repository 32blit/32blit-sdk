#pragma once

#include <stdint.h>

#include "32blit.hpp"

using namespace engine;
using namespace graphics;
using namespace utility;
using namespace input;

//extern surface fb;

void init();
void update(uint32_t time);
void render(uint32_t time);
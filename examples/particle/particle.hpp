#pragma once

#include <cstdint>

#include "32blit.hpp"
#include "engine/particle.hpp"

void init();
void update(uint32_t time);
void render(uint32_t time);
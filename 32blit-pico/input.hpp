#pragma once

#include <cstdint>

#include "types/vec2.hpp"

struct InputDriver {
  void (*init)();
  void (*update)(uint32_t &, blit::Vec2 &);
};

void init_input();
void update_input();

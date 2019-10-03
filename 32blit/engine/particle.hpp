#pragma once

#include <queue>
#include <functional>
#include <stdint.h>
#include "../types/vec2.hpp"

struct particle {
  vec2 pos;
  vec2 vel;  
  float age;

  uint32_t age_ms = 0;

  particle(vec2 pos, vec2 vel) : pos(pos), vel(vel) {};
};

struct generator {
  uint32_t count;
  uint32_t lifetime_ms;
  uint32_t generate_per_ms;
  uint32_t remaining_ms;
  vec2 force;

  std::function<particle*(void)> generate;

  std::deque<particle *> particles;

  generator(uint32_t count, uint32_t lifetime_ms, std::function<particle *(void)> generate);
  ~generator();

  void update(uint32_t time_ms);
};


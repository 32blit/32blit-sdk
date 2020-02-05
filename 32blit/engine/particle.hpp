#pragma once

#include <queue>
#include <functional>
#include <stdint.h>
#include "../types/vec2.hpp"

struct Particle {
  Vec2 pos;
  Vec2 vel;  
  float age;

  uint32_t age_ms = 0;

  Particle(Vec2 pos, Vec2 vel) : pos(pos), vel(vel) {};
};

struct ParticleGenerator {
  uint32_t count;
  uint32_t lifetime_ms;
  uint32_t generate_per_ms;
  uint32_t remaining_ms;
  Vec2 force;

  std::function<Particle*(void)> generate;

  std::deque<Particle *> particles;

  ParticleGenerator(uint32_t count, uint32_t lifetime_ms, std::function<Particle *(void)> generate);
  ~ParticleGenerator();

  void update(uint32_t time_ms);
};


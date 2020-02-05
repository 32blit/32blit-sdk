/*! \file particle.cpp
    \brief Particle system
*/
#include "particle.hpp"
#include "engine.hpp"

  /**
   * Create a new particle generator.
   *
   * \param count Number of particles in system.
   * \param lifetime_ms Particle lifetime in milliseconds.
   * \param generate callback function for generating particles.
   */
  ParticleGenerator::ParticleGenerator(uint32_t count, uint32_t lifetime_ms, std::function<Particle*(void)> generate) : count(count), lifetime_ms(lifetime_ms), generate(generate) {    
  }

  ParticleGenerator::~ParticleGenerator() {
    for (auto p : particles) {
      delete p;
    }
  }

  /**
   * Update state of the particle system.
   *
   * \param time_ms Particle system age, in milliseconds.
   */
  void ParticleGenerator::update(uint32_t time_ms) {
    static uint32_t last_time_ms = time_ms;
    uint32_t elapsed_ms = time_ms - last_time_ms;
    
    // delete expired particles
    while (particles.size() > 0 && particles.front()->age_ms > lifetime_ms) {
      delete particles.front();
      particles.pop_front();
    }
    
    // update remaining particles
    float td = (time_ms - last_time_ms) / 1000.0f;
    Vec2 f = force * td;
    for (auto p : particles) {
      p->vel += f;
      p->pos += p->vel * td;
      p->age_ms += elapsed_ms;
      p->age = p->age_ms / float(lifetime_ms);
    }

    // generate new particles to fill the queue
    uint32_t target_count = 1;
    if (particles.size() > 0) {
      target_count = (particles.front()->age_ms * count) / lifetime_ms;
    }
    while (particles.size() < target_count) {
      particles.push_back(generate());
    }

    last_time_ms = time_ms;
  }
#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "particle.hpp"

using namespace engine;
using namespace graphics;



/* setup */
void init() {
  engine::set_screen_mode(screen_mode::hires);
}

struct test_particle {
  vec2 pos;
  vec2 vel;
  int age;
  bool generated = false;
};


struct smoke_particle : particle {
  float age_boost;

  smoke_particle(vec2 pos, vec2 vel, float age_boost) : particle(pos, vel), age_boost(age_boost) {};
};

particle* generate_smoke() {
  return new smoke_particle(
    vec2((rand() % 20) - 10, (rand() % 20) - 10),
    vec2(((rand() % 40) - 20) / 2, (rand() % 20) - 60),
    (rand() % 3000) / 3000.0f
  );
}

generator smoke_generator(150, 4000, generate_smoke);

void render_smoke() {
  for (auto p : smoke_generator.particles) {
    smoke_particle *sp = static_cast<smoke_particle*>(p);
    float age = sp->age + sp->age_boost;
    int alpha = (255 - (age * 75.0f)) / 8.0f;
    int radius = (age * 150.0f) / 16.0f;
    fb.pen(rgba(255, 255, 255, alpha));
    fb.circle(p->pos + point(50, 240), radius);
  }
}

/*
void smoke_generate(test_particle &p) {
  p.pos = vec2((rand() % 20) - 10, (rand() % 20) - 10);
  p.vel = vec2(((rand() % 40) - 20) / 2, (rand() % 20) - 60);
  p.age = rand() % 200;
  p.generated = true;
};

void smoke(uint32_t time_ms) {
  static test_particle s[150];
  static int generate_index = 0;
  static uint32_t last_time_ms = time_ms;

  int elapsed_ms = time_ms - last_time_ms;
  float td = (elapsed_ms) / 1000.0f;

  smoke_generate(s[generate_index++]);
  if (generate_index >= 150)
    generate_index = 0;

  float w = sin(time_ms / 1000.0f) * 0.05f;

  for (auto &p : s) {
    if (p.generated) {
      p.pos += p.vel * td;
      p.vel += vec2(w, 0);
      p.age++;

      int alpha = (255 - (p.age / 2)) / 8.0f;
      int radius = (p.age) / 16.0f;
      fb.pen(rgba(255, 255, 255, alpha));
      fb.circle(p.pos + point(50, 240), radius);
    }
  }

  
  last_time_ms = time_ms;
};
*/

void spark_generate(test_particle &p) {
  p.pos = vec2((rand() % 10) - 5, -100);
  p.vel = vec2(((rand() % 40) - 20), (rand() % 80) - 70);
  p.age = 0;// rand() % 255;
  p.generated = true;
};

void spark(uint32_t time_ms) {
  static test_particle s[500];
  static int generate_index = 0;
  static uint32_t last_time_ms = time_ms;

  int elapsed_ms = time_ms - last_time_ms;
  float td = (elapsed_ms) / 1000.0f;

  spark_generate(s[generate_index++]);
  if (generate_index >= 500)
    generate_index = 0;

  float w = sin(time_ms / 1000.0f) * 0.05f;

  vec2 gravity = vec2(0, 9.8 * 2) * td;

  for (auto &p : s) {
    if (p.generated) {
      p.vel += gravity;
      p.pos += p.vel * td;
      if (p.pos.y > 0)
        p.vel.y *= -0.7;
      p.age++;

      int a = p.age / 2;
      int r = 255 - (p.age / 2);
      int g = 255 - (a * 2);
      g = g < 0 ? 0 : g;
      int b = 255 - (a * 4);
      b = b < 0 ? 0 : b;

      int bloom = (255 - a) / 64;
      fb.pen(rgba(r, g, b, 16));
      fb.circle(p.pos + point(160, 240), bloom);
      fb.pen(rgba(r, g, b, 16));
      fb.circle(p.pos + point(160, 240), bloom / 2);

      fb.pen(rgba(r, g, b));
      fb.pixel(p.pos + point(160, 240));

    }
  }
  
  last_time_ms = time_ms;
};


void rain_generate(test_particle &p) {
  p.pos = vec2((rand() % 80) - 40, (rand() % 10) - 250);
  p.vel = vec2(0, 140);
  p.age = 0;// rand() % 255;
  p.generated = true;
};

vec2 g = vec2(0, 9.8 * 5);

void rain(uint32_t time_ms) {
  static test_particle s[200];
  static int generate_index = 0;
  static uint32_t last_time_ms = time_ms;

  int elapsed_ms = time_ms - last_time_ms;
  float td = (elapsed_ms) / 1000.0f;

  rain_generate(s[generate_index++]);
  if (generate_index >= 200)
    generate_index = 0;

  float w = sin(time_ms / 1000.0f) * 0.05f;

  vec2 gravity = g * td;

  for (auto &p : s) {
    if (p.generated) {
      p.vel += gravity;
      p.pos += p.vel * td;
      
      int floor = -3;
      if (p.pos.x > 20 && p.pos.x < 46)
        floor = -33;

      if (p.pos.y >= floor) {
        p.pos.y = floor;
        float bounce = ((rand() % 10)) / 80.0f;
        p.vel.y *= -bounce;
        p.vel.x = ((rand() % 30) - 15);
      }
      p.age++;

      int a = p.age / 2;
      int r = 150 - (a / 2);
      int g = 150 - (a / 2);
      int b = 255;// -(a * 4);

      if(p.vel.length() > 20) {
        fb.pen(rgba(r, g, b, 100));
        fb.pixel(p.pos + point(270, 239));
        fb.pen(rgba(r, g, b, 160));
        fb.pixel(p.pos + point(270, 241));
      }
      fb.pen(rgba(r, g, b, 180));
      fb.pixel(p.pos + point(270, 242));
    }
  }

  last_time_ms = time_ms;
};



particle* generate_basic_rain() {  
  return new particle(
    vec2((std::rand() % 120) + 100, 0),
    vec2(0, 100)
  );
}

generator basic_rain_generator(250, 4000, generate_basic_rain);

void render_basic_rain() {
  for (auto p : basic_rain_generator.particles) {
    particle *sp = static_cast<particle*>(p);
    if (sp->pos.y >= 239) {
      sp->pos.y = 239;
    }
    fb.pen(rgba(128, 128, 255));
    fb.pixel(sp->pos);
  }
}


void render(uint32_t time_ms) {

  fb.pen(rgba(0, 0, 0, 255));
  fb.clear();

  //render_smoke();

  uint32_t ms_start = now();
  //spark(time_ms);
  //smoke(time_ms); 
  //rain(time_ms);
  render_basic_rain();
  uint32_t ms_end = now();

  // draw grid
  fb.alpha = 255;
  fb.pen(rgba(255, 255, 255));
  fb.rectangle(rect(0, 0, 320, 14));
  fb.pen(rgba(0, 0, 0));
  fb.text("Rain demo", &minimal_font[0][0], point(5, 4));

 /* fb.pen(rgba(255, 255, 255));  
  fb.text("Smoke:", &minimal_font[0][0], point(10, 20));
  fb.text("Sparks:", &minimal_font[0][0], point(120, 20));
  fb.text("Rain:", &minimal_font[0][0], point(220, 20));  */

  // draw FPS meter
  /*fb.alpha = 255;
  fb.pen(rgba(0, 0, 0));
  fb.rectangle(rect(1, 240 - 10, 12, 9));
  fb.pen(rgba(255, 255, 255, 200));
  std::string fms = std::to_string(ms_end - ms_start);
  fb.text(fms, &minimal_font[0][0], rect(3, 240 - 9, 10, 16));

  int block_size = 4;
  for (int i = 0; i < (ms_end - ms_start); i++) {
    fb.pen(rgba(i * 5, 255 - (i * 5), 0));
    fb.rectangle(rect(i * (block_size + 1) + 1 + 13, fb.bounds.h - block_size - 1, block_size, block_size));
  }*/

  fb.watermark();
}

void update(uint32_t time_ms) {
//  smoke_generator.update(time_ms);
  basic_rain_generator.update(time_ms);

  if (pressed(input::DPAD_LEFT)) {
    g.rotate(0.1f);
  }
}
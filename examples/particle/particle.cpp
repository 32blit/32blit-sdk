#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "particle.hpp"

using namespace blit;



/* setup */
void init() {
  blit::set_screen_mode(ScreenMode::hires);
}

struct test_particle {
  Vec2 pos;
  Vec2 vel;
  int age;
  bool generated = false;
};


struct smoke_particle : Particle {
  float age_boost;

  smoke_particle(Vec2 pos, Vec2 vel, float age_boost) : Particle(pos, vel), age_boost(age_boost) {};
};

Particle* generate_smoke() {
  return new smoke_particle(
    Vec2((rand() % 20) - 10, (rand() % 20) - 10),
    Vec2(((rand() % 40) - 20) / 2, (rand() % 20) - 60),
    (rand() % 3000) / 3000.0f
  );
}

ParticleGenerator smoke_generator(150, 4000, generate_smoke);

void render_smoke() {
  for (auto p : smoke_generator.particles) {
    smoke_particle *sp = static_cast<smoke_particle*>(p);
    float age = sp->age + sp->age_boost;
    int alpha = (255 - (age * 75.0f)) / 8.0f;
    int radius = (age * 150.0f) / 16.0f;
    screen.pen = Pen(255, 255, 255, alpha);
    screen.circle(p->pos + Point(50, 240), radius);
  }
}


void smoke_generate(test_particle &p) {
  p.pos = Vec2((rand() % 20) - 10, (rand() % 20) - 10);
  p.vel = Vec2(((rand() % 40) - 20) / 2, (rand() % 20) - 60);
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
      p.vel += Vec2(w, 0);
      p.age++;

      int alpha = (255 - (p.age / 2)) / 8.0f;
      int radius = (p.age) / 16.0f;
      screen.pen = Pen(255, 255, 255, alpha);
      screen.circle(p.pos + Point(50, 240), radius);
    }
  }

  
  last_time_ms = time_ms;
};


void spark_generate(test_particle &p) {
  p.pos = Vec2((rand() % 10) - 5, -100);
  p.vel = Vec2(((rand() % 40) - 20), (rand() % 80) - 70);
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

  Vec2 gravity = Vec2(0, 9.8 * 2) * td;

  for (auto &p : s) {
    if (p.generated) {
      p.vel += gravity;
      p.pos += p.vel * td;
      if (p.pos.y > 0)
        p.vel.y *= -0.7f;
      p.age++;

      int a = p.age / 2;
      int r = 255 - (p.age / 2);
      int g = 255 - (a * 2);
      g = g < 0 ? 0 : g;
      int b = 255 - (a * 4);
      b = b < 0 ? 0 : b;

      int bloom = (255 - a) / 64;
      screen.pen = Pen(r, g, b, 16);
      screen.circle(p.pos + Point(160, 240), bloom);
      screen.pen = Pen(r, g, b, 16);
      screen.circle(p.pos + Point(160, 240), bloom / 2);

      screen.pen = Pen(r, g, b);
      screen.pixel(p.pos + Point(160, 240));

    }
  }
  
  last_time_ms = time_ms;
};


void rain_generate(test_particle &p) {
  p.pos = Vec2((rand() % 80) - 40, (rand() % 10) - 250);
  p.vel = Vec2(0, 140);
  p.age = 0;// rand() % 255;
  p.generated = true;
};

Vec2 g = Vec2(0, 9.8 * 5);

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

  Vec2 gravity = g * td;

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
        screen.pen = Pen(r, g, b, 100);
        screen.pixel(p.pos + Point(270, 239));
        screen.pen = Pen(r, g, b, 160);
        screen.pixel(p.pos + Point(270, 241));
      }
      screen.pen = Pen(r, g, b, 180);
      screen.pixel(p.pos + Point(270, 242));
    }
  }

  last_time_ms = time_ms;
};



Particle* generate_basic_rain() {  
  return new Particle(
    Vec2((std::rand() % 120) + 100, 0),
    Vec2(0, 100)
  );
}

ParticleGenerator basic_rain_generator(250, 4000, generate_basic_rain);

void render_basic_rain() {
  for (auto p : basic_rain_generator.particles) {
    Particle *sp = static_cast<Particle*>(p);
    if (sp->pos.y >= 239) {
      sp->pos.y = 239;
    }
    screen.pen = Pen(128, 128, 255);
    screen.pixel(sp->pos);
  }
}

uint32_t prev_buttons = blit::buttons;

void render(uint32_t time_ms) {

uint16_t width;
uint16_t height;
    if ((blit::buttons ^ prev_buttons) & blit::Button::A) {
        blit::set_screen_mode(blit::lores);
        width = 160;
        height = 120;
    }
    else if ((blit::buttons ^ prev_buttons) & blit::Button::B) {
        blit::set_screen_mode(blit::hires);
        width = 320;
        height = 240;
    }
    prev_buttons = blit::buttons;

  screen.pen = Pen(0, 0, 0, 255);
  screen.clear();

  //render_smoke();

  uint32_t ms_start = now();
  spark(time_ms);
  smoke(time_ms); 
  rain(time_ms);
  //render_basic_rain();
  uint32_t ms_end = now();

  // draw grid
  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));
  screen.pen = Pen(0, 0, 0);
  screen.text("Rain demo", minimal_font, Point(5, 4));

 /* screen.pen = Pen(255, 255, 255);  
  screen.text("Smoke:", minimal_font, point(10, 20));
  screen.text("Sparks:", minimal_font, point(120, 20));
  screen.text("Rain:", minimal_font, point(220, 20));  */

  // draw FPS meter
  /*screen.alpha = 255;
  screen.pen = Pen(0, 0, 0);
  screen.rectangle(rect(1, 240 - 10, 12, 9));
  screen.pen = Pen(255, 255, 255, 200);
  std::string fms = std::to_string(ms_end - ms_start);
  screen.text(fms, minimal_font, rect(3, 240 - 9, 10, 16));

  int block_size = 4;
  for (int i = 0; i < (ms_end - ms_start); i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(rect(i * (block_size + 1) + 1 + 13, screen.bounds.h - block_size - 1, block_size, block_size));
  }*/

  screen.watermark();
}

void update(uint32_t time_ms) {
//  smoke_generator.update(time_ms);
  basic_rain_generator.update(time_ms);

  if (pressed(Button::DPAD_LEFT)) {
    g.rotate(0.1f);
  }
}
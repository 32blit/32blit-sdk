#include <string>
#include <cstring>
#include <memory>
#include <cstdlib>

#include "tilt.hpp"

using namespace blit;

#define GRAIN_COUNT 5000


uint8_t grain_mask[160 * 120];

bool is_occupied(const Point &p) {
  return grain_mask[p.x + (p.y * 160)] != 0;
}

void set_occupied(const Point &p) {
  grain_mask[p.x + (p.y * 160)] = 1;
}

void clear_occupied(const Point &p) {
  grain_mask[p.x + (p.y * 160)] = 0;
}

float deg2rad(float a) {
  return a * (pi / 180.0f);
}

Vec2 gravity(0, 1000.0f);

Pen colours[] = {
  Pen(7, 254, 9),
  Pen(230, 194, 41),
  Pen(239, 45, 86),
  Pen(241, 113, 5),
  Pen(26, 143, 227),
  Pen(201, 26, 227),
};

struct grain {
  bool test_move(const Point &tp) {
    if (screen.bounds.contains(tp)) {
      if ( (tp.x == (int)p.x && tp.y == (int)p.y) ||
           !is_occupied(tp) ) {
        return true;
      }
    }

    return false;
  }

  bool update(const float td) {
    v += (gravity * td);
    if (v.length() > 50.0f) {
      v.normalize();
      v *= 50.0f;
    }

    // test point is along the velocity vector
    Vec2 np = p;
    Vec2 tp = p + (v * td);
    bool found = false;
    if ((found = test_move(tp))) {
      np = tp;
    } else {
      // optimised -45 degree rotations
      float rx = v.x *  0.70710f - v.y * -0.70710f;
      float ry = v.x * -0.70710f + v.y *  0.70710f;
      v.x = rx;
      v.y = ry;

      tp = p + (v * td);

      if ((found = test_move(tp))) {
        np = tp;
      }
      else {
        // optimised 90 degree rotations
        float rx = -v.y;
        float ry = v.x;
        v.x = rx;
        v.y = ry;

        tp = p + (v * td);

        if ((found = test_move(tp))) {
          np = tp;
        }
      }
    }

    // update the position of our grain
    if (found) {
      clear_occupied(p);
      p = np;
      set_occupied(np);
    }
    else {
      v = Vec2(0, 0);
    }

    return true;
  }

public:
  Vec2 p = Vec2(0, 0);
  Vec2 v = Vec2(0, 1);
  uint8_t c = 0;
};

grain grains[GRAIN_COUNT];


/* setup */
void init() {
  for (auto &g : grains) {
    do {
      g.p = Vec2(Vec2(rand() % 160, rand() % 120));
      g.c = rand() % 6;
    } while (is_occupied(g.p));    
  }
}

uint32_t update_time_ms = 0;

void render(uint32_t time_ms) {

  screen.pen = Pen(0, 0, 0, 255);
  screen.clear();

  for (auto &g : grains) {
    screen.pen = colours[g.c];
    screen.pixel(g.p);
  }

  screen.pen = Pen(255, 255, 255);
  Point centre = Point(80, 60);
  screen.line(centre, centre + (gravity * 20.0f));  

  screen.watermark();


  // draw FPS meter
  screen.alpha = 255;
  screen.pen = Pen(0, 0, 0);
  screen.rectangle(Rect(1, 120 - 10, 12, 9));
  screen.pen = Pen(255, 255, 255, 200);
  std::string fms = std::to_string(update_time_ms);
  screen.text(fms, minimal_font, Rect(3, 120 - 9, 10, 16));

  int block_size = 4;
  for (uint32_t i = 0; i < update_time_ms; i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(Rect(i * (block_size + 1) + 1 + 13, screen.bounds.h - block_size - 1, block_size, block_size));
  }
}


void update(uint32_t time_ms) {
  static uint32_t last_time_ms = time_ms;

  //  smoke_generator.update(time_ms);
 
 /*
  if (pressed(button::DPAD_LEFT)) {
    gravity.rotate(0.01f);
  }

  if (pressed(button::DPAD_RIGHT)) {
    gravity.rotate(-0.01f);
  }

  if (pressed(button::DPAD_UP)) {
    gravity *= 1.01f;
  }

  if (pressed(button::DPAD_DOWN)) {
    gravity /= 1.01f;
  }
  */

  gravity.x = blit::tilt.x;
  gravity.y = blit::tilt.y;
  gravity *= 1000.0f;

  float td = (time_ms - last_time_ms) / 1000.0f;

  uint32_t ms_start = now();
  for (auto &g : grains) {    
    g.update(td);
  }
  uint32_t ms_end = now();
  update_time_ms = ms_end - ms_start;

  last_time_ms = time_ms;
}
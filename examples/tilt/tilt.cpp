#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "tilt.hpp"

using namespace blit;

#define GRAIN_COUNT 5000


uint8_t grain_mask[160 * 120];

bool is_occupied(const point &p) {
  return grain_mask[p.x + (p.y * 160)] != 0;
}

void set_occupied(const point &p) {
  grain_mask[p.x + (p.y * 160)] = 1;
}

void clear_occupied(const point &p) {
  grain_mask[p.x + (p.y * 160)] = 0;
}

float deg2rad(float a) {
  return a * (M_PI / 180.0f);
}

vec2 gravity(0, 1000.0f);

rgba colours[] = {
  rgba(7, 254, 9),
  rgba(230, 194, 41),
  rgba(239, 45, 86),
  rgba(241, 113, 5),
  rgba(26, 143, 227),
  rgba(201, 26, 227),
};

struct grain {
  bool test_move(const point &tp) {
    if (fb.bounds.contains(tp)) {
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
    vec2 np = p;
    vec2 tp = p + (v * td);
    bool found = false;
    if (found = test_move(tp)) {
      np = tp;
    } else {
      // optimised -45 degree rotations
      float rx = v.x *  0.70710f - v.y * -0.70710f;
      float ry = v.x * -0.70710f + v.y *  0.70710f;
      v.x = rx;
      v.y = ry;

      tp = p + (v * td);

      if (found = test_move(tp)) {
        np = tp;
      }
      else {
        // optimised 90 degree rotations
        float rx = -v.y;
        float ry = v.x;
        v.x = rx;
        v.y = ry;

        tp = p + (v * td);

        if (found = test_move(tp)) {
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
      v = vec2(0, 0);
    }

    return true;
  }

public:
  vec2 p = vec2(0, 0);
  vec2 v = vec2(0, 1);
  uint8_t c = 0;
};

grain grains[GRAIN_COUNT];


/* setup */
void init() {
  for (auto &g : grains) {
    do {
      g.p = vec2(vec2(rand() % 160, rand() % 120));
      g.c = rand() % 6;
    } while (is_occupied(g.p));    
  }
}

uint32_t update_time_ms = 0;

void render(uint32_t time_ms) {

  fb.pen(rgba(0, 0, 0, 255));
  fb.clear();

  for (auto &g : grains) {
    fb.pen(colours[g.c]);
    fb.pixel(g.p);
  }

  fb.pen(rgba(255, 255, 255));
  point centre = point(80, 60);
  fb.line(centre, centre + (gravity * 20.0f));  

  fb.watermark();


  // draw FPS meter
  fb.alpha = 255;
  fb.pen(rgba(0, 0, 0));
  fb.rectangle(rect(1, 120 - 10, 12, 9));
  fb.pen(rgba(255, 255, 255, 200));
  std::string fms = std::to_string(update_time_ms);
  fb.text(fms, &minimal_font[0][0], rect(3, 120 - 9, 10, 16));

  int block_size = 4;
  for (int i = 0; i < update_time_ms; i++) {
    fb.pen(rgba(i * 5, 255 - (i * 5), 0));
    fb.rectangle(rect(i * (block_size + 1) + 1 + 13, fb.bounds.h - block_size - 1, block_size, block_size));
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
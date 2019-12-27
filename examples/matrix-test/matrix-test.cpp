#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "matrix-test.hpp"

using namespace blit;

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;

/* define storage for the framebuffer, spritesheet, and mask */
//rgb     __fb[screen_width * screen_height] __attribute__((section(".fb")));
#ifdef __APPLE__
uint8_t __m[screen_width * screen_height] __attribute__((section("__DATA,.m")));
uint8_t __pb[screen_width * screen_height] __attribute__((section("__DATA,.fb")));
#else
uint8_t __m[screen_width * screen_height] __attribute__((section(".m")));
uint8_t __pb[screen_width * screen_height] __attribute__((section(".fb")));
#endif

/* create surfaces */
//surface fb((uint8_t *)__fb, size(screen_width, screen_height), pixel_format::RGB);
surface m((uint8_t *)__m, pixel_format::M, size(screen_width, screen_height));
surface fbb((uint8_t *)__pb, pixel_format::P, size(screen_width, screen_height));

/* setup */
void init() {
  /*
      fb.alpha = 255;
      fb.mask = nullptr;
      fb.pen(rgba(39, 39, 54));
      fb.clear();*/
}


float angle = 0.0f;

float deg2rad(float a) {
  return a * (M_PI / 180.0f);
}

void draw(std::array<vec2, 4> vecs, std::vector<mat3> trs) {
  int s = (150) / trs.size();
  rgba p(255, 255, 255, 32);

  auto o = vec2(80, 60);

  fb.pen(p);
  fb.line(vecs[0] + o, vecs[1] + o);
  fb.line(vecs[1] + o, vecs[2] + o);
  fb.line(vecs[2] + o, vecs[3] + o);
  fb.line(vecs[3] + o, vecs[0] + o);


  for (auto &tr : trs) {
    p.a += s;

    for (auto &v : vecs) {
      v *= tr;
    }

    auto o = vec2(80, 60);

    fb.pen(p);
    fb.line(vecs[0] + o, vecs[1] + o);
    fb.line(vecs[1] + o, vecs[2] + o);
    fb.line(vecs[2] + o, vecs[3] + o);
    fb.line(vecs[3] + o, vecs[0] + o);
  }

  // then apply the translations in reverse and ensure we end up back at the start
  for (int i = trs.size() - 1; i >= 0; i--) {
    mat3 tr = trs[i];

    tr.inverse();
    for (auto &v : vecs) {
      v *= tr;
    }

    fb.pen(rgba(255, 0, 0));
    fb.line(vecs[0] + o, vecs[1] + o);
    fb.line(vecs[1] + o, vecs[2] + o);
    fb.line(vecs[2] + o, vecs[3] + o);
    fb.line(vecs[3] + o, vecs[0] + o);
  }
}

void render(uint32_t time_ms) {
  
  fb.pen(rgba(20, 30, 40));
  fb.clear();

  std::array<vec2, 4> vecs = {
    vec2(-10, -10),
    vec2(10, -10),
    vec2(10, 10),
    vec2(-10, 10)
  };

  draw(
    vecs,       
    { mat3::translation(vec2(20, 20)),
      mat3::scale(vec2(2, 2)),
      mat3::rotation(angle),
      mat3::translation(vec2(10, 10)) 
    }
  );
  /*
  draw(
    vecs,
    mat3::scale(vec2(2, 2)) *
    mat3::translation(vec2(10, 10)) *
    mat3::rotation(angle)
  );*/

  

//    fb.text("angle: " + std::to_string(int(angle)), &minimal_font[0][0], rect(0, 0, 160, 10));


}

void update(uint32_t time) {
  angle++;
}
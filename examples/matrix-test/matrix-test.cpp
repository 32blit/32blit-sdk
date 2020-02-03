#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "matrix-test.hpp"

using namespace blit;

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;

/* define storage for the framebuffer, spritesheet, and mask */
//rgb     __fb[screen_width * screen_height] __SECTION__(".fb");
uint8_t __m[screen_width * screen_height] __SECTION__(".m");
uint8_t __pb[screen_width * screen_height] __SECTION__(".fb");

/* create surfaces */
//surface fb((uint8_t *)__fb, size(screen_width, screen_height), pixel_format::RGB);
Surface m((uint8_t *)__m, PixelFormat::M, Size(screen_width, screen_height));
Surface fbb((uint8_t *)__pb, PixelFormat::P, Size(screen_width, screen_height));

/* setup */
void init() {
  /*
      screen.alpha = 255;
      screen.mask = nullptr;
      screen.pen(rgba(39, 39, 54));
      screen.clear();*/
}


float angle = 0.0f;

float deg2rad(float a) {
  return a * (M_PI / 180.0f);
}

void draw(std::array<Vec2, 4> vecs, std::vector<Mat3> trs) {
  int s = (150) / trs.size();
  RGBA p(255, 255, 255, 32);

  auto o = Vec2(80, 60);

  screen.pen(p);
  screen.line(vecs[0] + o, vecs[1] + o);
  screen.line(vecs[1] + o, vecs[2] + o);
  screen.line(vecs[2] + o, vecs[3] + o);
  screen.line(vecs[3] + o, vecs[0] + o);


  for (auto &tr : trs) {
    p.a += s;

    for (auto &v : vecs) {
      v *= tr;
    }

    auto o = Vec2(80, 60);

    screen.pen(p);
    screen.line(vecs[0] + o, vecs[1] + o);
    screen.line(vecs[1] + o, vecs[2] + o);
    screen.line(vecs[2] + o, vecs[3] + o);
    screen.line(vecs[3] + o, vecs[0] + o);
  }

  // then apply the translations in reverse and ensure we end up back at the start
  for (int i = trs.size() - 1; i >= 0; i--) {
    Mat3 tr = trs[i];

    tr.inverse();
    for (auto &v : vecs) {
      v *= tr;
    }

    screen.pen(RGBA(255, 0, 0));
    screen.line(vecs[0] + o, vecs[1] + o);
    screen.line(vecs[1] + o, vecs[2] + o);
    screen.line(vecs[2] + o, vecs[3] + o);
    screen.line(vecs[3] + o, vecs[0] + o);
  }
}

void render(uint32_t time_ms) {
  
  screen.pen(RGBA(20, 30, 40));
  screen.clear();

  std::array<Vec2, 4> vecs = {
    Vec2(-10, -10),
    Vec2(10, -10),
    Vec2(10, 10),
    Vec2(-10, 10)
  };

  draw(
    vecs,       
    { Mat3::translation(Vec2(20, 20)),
      Mat3::scale(Vec2(2, 2)),
      Mat3::rotation(angle),
      Mat3::translation(Vec2(10, 10)) 
    }
  );
  /*
  draw(
    vecs,
    mat3::scale(vec2(2, 2)) *
    mat3::translation(vec2(10, 10)) *
    mat3::rotation(angle)
  );*/

  

//    screen.text("angle: " + std::to_string(int(angle)), &minimal_font[0][0], rect(0, 0, 160, 10));


}

void update(uint32_t time) {
  angle++;
}
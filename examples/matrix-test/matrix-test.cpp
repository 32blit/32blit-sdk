#include <string>
#include <cstring>
#include <memory>
#include <cstdlib>

#include "matrix-test.hpp"

using namespace blit;

/* setup */
void init() {
  /*
      screen.alpha = 255;
      screen.mask = nullptr;
      screen.pen = rgba(39, 39, 54);
      screen.clear();*/
}


float angle = 0.0f;

float deg2rad(float a) {
  return a * (pi / 180.0f);
}

void draw(std::array<Vec2, 4> vecs, std::vector<Mat3> trs) {
  auto s = uint8_t(150 / trs.size());
  Pen p(255, 255, 255, 32);

  auto o = Vec2(80, 60);

  screen.pen = p;
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

    screen.pen = p;
    screen.line(vecs[0] + o, vecs[1] + o);
    screen.line(vecs[1] + o, vecs[2] + o);
    screen.line(vecs[2] + o, vecs[3] + o);
    screen.line(vecs[3] + o, vecs[0] + o);
  }

  // then apply the translations in reverse and ensure we end up back at the start
  for (auto it = trs.rbegin(); it != trs.rend(); ++it) {
    Mat3 &tr = *it;

    tr.inverse();
    for (auto &v : vecs) {
      v *= tr;
    }

    screen.pen = Pen(255, 0, 0);
    screen.line(vecs[0] + o, vecs[1] + o);
    screen.line(vecs[1] + o, vecs[2] + o);
    screen.line(vecs[2] + o, vecs[3] + o);
    screen.line(vecs[3] + o, vecs[0] + o);
  }
}

void render(uint32_t time_ms) {

  screen.pen = Pen(20, 30, 40);
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



//    screen.text("angle: " + std::to_string(int(angle)), minimal_font, rect(0, 0, 160, 10));


}

void update(uint32_t time) {
  angle++;
}

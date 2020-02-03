
#include <map>
#include <string>
#include <vector>
#include <cstdlib>

#include "logo.hpp"

using namespace blit;

const Size screen_size(160, 120);

/* define storage for the framebuffer, spritesheet, and mask */
//rgb     __fb[160 * 120] __SECTION__(".fb")));
RGBA    __ss[128 * 128] __SECTION__(".ss");
uint8_t __m[160 * 120] __SECTION__(".m");

/* create surfaces */
//surface fb((uint8_t *)__fb, screen_size, pixel_format::RGB);
Surface ss((uint8_t *)__ss, PixelFormat::RGBA, Size(128, 128));
Surface m((uint8_t *)__m, PixelFormat::M, screen_size);

uint8_t logo[] = {
  1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2,
  0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2,
  1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2,
  0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 1, 0, 2, 0, 1, 0,
  1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1,
  2, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  2, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  2, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1
};

struct Particle {
  Vec3 direction;
  Vec3 position;
  RGBA color;
};

// convert the logo points into vec3
Particle particles[63];

/* setup */
void init() {
  int i = 0;
  for (int y = 0; y < 11; y++) {
    for (int x = 0; x < 11; x++) {
      int col = logo[x + y * 11];
      if (col != 0) {
        particles[i].color = col == 1 ? RGBA(255, 255, 255) : RGBA(0, 255, 0);
        particles[i].position = Vec3(x - 5.0f, y - 5.0f, 0.0f);

        float x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;
        float y = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) / 4.0f;
        float z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;

        particles[i].direction = Vec3(x, y, z);
        particles[i].direction.normalize();
        i++;
      }
    }
  }

  //engine::update = update;
  //engine::render = render;
}

float clamp(float v, float min, float max) {
  return v < min ? min : (v > max) ? max : v;

}

void render(uint32_t time) {
  static int32_t x = 0; x++;
  static float ts = 0.0f;
  static float dir = 0.02f;

  //screen.pen(rgba(39, 39, 54, 100));
  screen.pen(RGBA(0, 0, 0, 50));
  screen.clear();

  uint32_t ms_start = now();

  time = time % 10000;
  float duration = 2.0f;

  ts += dir;
  if (ts > 4.0f) {     
    dir *= -1;
    ts = 1.0f;
  }

  if (ts <= 0.0f) {
    dir *= -1;
  }

  float speed = 125.0f;
  float td = (1.0f - std::min(1.0f, ts));

  screen.alpha = 255;

  float scale = 7;
  float half_scale = scale / 2.0f + 0.5f;
  Mat4 r = Mat4::rotation(td * 250.0f, Vec3(0, 1, 0));
  Mat4 s = Mat4::scale(Vec3(scale, scale, scale));
  Mat4 t = Mat4::translation(Vec3(80, 60, 0));

  for (int i = 0; i < 63; i++) {
    Particle pc = particles[i];
    pc.position *= s;
    pc.position += pc.direction * td * speed;


    Vec3 tl = pc.position;
    tl.x -= half_scale;
    tl.y -= half_scale;

    Vec3 tr = pc.position;
    tr.x += half_scale;
    tr.y -= half_scale;

    Vec3 bl = pc.position;
    bl.x -= half_scale;
    bl.y += half_scale;

    Vec3 br = pc.position;
    br.x += half_scale;
    br.y += half_scale;

    float pcs = 50.0f;
    tl *= r;
    float tlp = 1.0f + clamp(tl.z / pcs, 0.0f, 1.0f);
    tl *= tlp;
    tl *= t;

    tr *= r;
    float trp = 1.0f + clamp(tr.z / pcs, 0.0f, 1.0f);
    tr *= trp;
    tr *= t;

    bl *= r;
    float blp = 1.0f + clamp(bl.z / pcs, 0.0f, 1.0f);
    bl *= blp;
    bl *= t;

    br *= r;
    float brp = 1.0f + clamp(br.z / pcs, 0.0f, 1.0f);
    br *= brp;
    br *= t;


    //float fade = (pc.position.z / 10.0f) + 1.0f;
    screen.pen(pc.color);// rgba(pc.color.r * fade, pc.color.g * fade, pc.color.b * fade);
    //g.pen.a = brp * 127.0f;


    Point ptl(tl.x, tl.y);
    Point ptr(tr.x, tr.y);
    Point pbr(br.x, br.y);
    Point pbl(bl.x, bl.y);
    Point pts[4] = { ptl, ptr, pbr, pbl };

    //line(ptl, ptr);
    //line(ptr, pbr);
    //line(pbr, pbl);
    //line(pbl, ptl);

    //polygon(pts, 4);

    screen.triangle(ptl, ptr, pbl);
    screen.triangle(pbl, ptr, pbr);
    /*    rectangle(rect(
          pc.position.x - half_scale + 80,
          pc.position.y -half_scale + 60,
          scale,
          scale));*/

  }


}

void update(uint32_t time) {
  float duration = 0.01f;     // == 10ms as a float
}
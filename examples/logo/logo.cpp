
#include <map>
#include <string>
#include <vector>
#include <cstdlib>

#include "logo.hpp"

using namespace blit;

const size screen_size(160, 120);

/* define storage for the framebuffer, spritesheet, and mask */
//rgb     __fb[160 * 120] __SECTION__(".fb")));
rgba    __ss[128 * 128] __SECTION__(".ss");
uint8_t __m[160 * 120] __SECTION__(".m");

/* create surfaces */
//surface fb((uint8_t *)__fb, screen_size, pixel_format::RGB);
surface ss((uint8_t *)__ss, pixel_format::RGBA, size(128, 128));
surface m((uint8_t *)__m, pixel_format::M, screen_size);

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

struct particle {
  vec3 direction;
  vec3 position;
  rgba color;
};

// convert the logo points into vec3
particle particles[63];

/* setup */
void init() {
  int i = 0;
  for (int y = 0; y < 11; y++) {
    for (int x = 0; x < 11; x++) {
      int col = logo[x + y * 11];
      if (col != 0) {
        particles[i].color = col == 1 ? rgba(255, 255, 255) : rgba(0, 255, 0);
        particles[i].position = vec3(x - 5.0f, y - 5.0f, 0.0f);

        float x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;
        float y = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) / 4.0f;
        float z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;

        particles[i].direction = vec3(x, y, z);
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

  //fb.pen(rgba(39, 39, 54, 100));
  fb.pen(rgba(0, 0, 0, 50));
  fb.clear();

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

  fb.alpha = 255;

  float scale = 7;
  float half_scale = scale / 2.0f + 0.5f;
  mat4 r = mat4::rotation(td * 250.0f, vec3(0, 1, 0));
  mat4 s = mat4::scale(vec3(scale, scale, scale));
  mat4 t = mat4::translation(vec3(80, 60, 0));

  for (int i = 0; i < 63; i++) {
    particle pc = particles[i];
    pc.position *= s;
    pc.position += pc.direction * td * speed;


    vec3 tl = pc.position;
    tl.x -= half_scale;
    tl.y -= half_scale;

    vec3 tr = pc.position;
    tr.x += half_scale;
    tr.y -= half_scale;

    vec3 bl = pc.position;
    bl.x -= half_scale;
    bl.y += half_scale;

    vec3 br = pc.position;
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
    fb.pen(pc.color);// rgba(pc.color.r * fade, pc.color.g * fade, pc.color.b * fade);
    //g.pen.a = brp * 127.0f;


    point ptl(tl.x, tl.y);
    point ptr(tr.x, tr.y);
    point pbr(br.x, br.y);
    point pbl(bl.x, bl.y);
    point pts[4] = { ptl, ptr, pbr, pbl };

    //line(ptl, ptr);
    //line(ptr, pbr);
    //line(pbr, pbl);
    //line(pbl, ptl);

    //polygon(pts, 4);

    fb.triangle(ptl, ptr, pbl);
    fb.triangle(pbl, ptr, pbr);
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
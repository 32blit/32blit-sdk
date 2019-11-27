
#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "graphics/mode7.hpp"


#include "flight.hpp"

using namespace blit;

uint8_t logo[16][16] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 0, 0, 0, 0, 1, 0, 2, 0, 1, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 2, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0},
  {0, 0, 2, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0},
  {0, 0, 2, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

const uint16_t screen_width = 160;
const uint16_t screen_height = 120;

/* define storage for the framebuffer, spritesheet, and mask */
uint8_t __mask[screen_width * screen_height] __attribute__((section(".m")));
// extra space allocated to take mipmaps
uint8_t __sprites[(screen_width * screen_height) + (64 * 128 * 4)] __attribute__((section(".fb")));
uint8_t __water[64 * 64] __attribute__((section(".fb")));

/* create surfaces */
surface mask((uint8_t *)__mask, pixel_format::M, size(screen_width, screen_height));
spritesheet *sprites;
spritesheet *water;

Map map(rect(0, 0, 128, 128));


struct object {
  vec2 pos;
  uint8_t type;

  object(vec2 pos, uint8_t type) : pos(pos), type(type) {}
};

std::vector<object> objects;

/* setup */
void init() {
  //set_screen_mode(screen_mode::hires);

  map.add_layer("ground", layer);
  map.layers["ground"].transforms = layer_transforms;

  sprites = spritesheet::load(packed_data);
  sprites->generate_mipmaps(3);

  water = spritesheet::load(water_packed_data);

  // extract information about objects from the map data
  point p;
  for (p.y = 0; p.y < 128; p.y++) {
    for (p.x = 0; p.x < 128; p.x++) {
      int16_t tid = map.layers["ground"].tile_at(p);
      if (tid == 27) {
        objects.emplace_back(
          vec2(p.x * 8 + 4, p.y * 8 + 4),
          1
        );
      }
      if (tid == 28) {
        objects.emplace_back(
          vec2(p.x * 8 + 4, p.y * 8 + 4),
          2
        );
      }
      if (tid == 29) {
        objects.emplace_back(
          vec2(p.x * 8 + 4, p.y * 8 + 4),
          3
        );
      }
      if (tid == 30) {
        objects.emplace_back(
          vec2(p.x * 8 + 4, p.y * 8 + 4),
          4
        );
      }
    }
  }

  /*
// draw objects

fb.alpha = 255;*/

}

static vec2 vel(0, 0);
static float angle = -15.0f * (M_PI / 180.0f);
vec2 pos(512, 512);

float map_size = 128;
float fov = 95.0f * (M_PI / 180.0f);
float far = 500.0f;
float near = 10.0f;

float deg2rad(float a) {
  return a * (M_PI / 180.0f);
}

float rad2deg(float r) {
  return r * (180.0f / M_PI);
}


void render(uint32_t time_ms) {
  static int tick_count = 0; tick_count++;


  fb.alpha = 255;
  fb.mask = nullptr;
  fb.pen(rgba(99, 155, 255));
  fb.rectangle(rect(0, 0, 160, 50));

  fb.pen(rgba(91, 110, 225));
  fb.rectangle(rect(0, 50, 160, 120 - 50));

  uint32_t ms_start = now();

  rect vp(0, 50, 160, 120 - 50);
  
  fb.alpha = 55;

  fb.blit(water, rect(0, 0, 64, 64), point(0, 50));
  /*
  blend_blit_func bbf = fb.bbf[water.format];
  for (int y = 50; y < 120; y++) {
    for (int x = 0; x < 160; x++) {
      point tx(x, y);      
      tx.x %= 64;
      tx.y %= 64;

      bbf(&water, water.offset(tx), &fb, fb.offset(x, y), 1, 1);
    }
  }*/

  mode7(&fb, sprites, &map.layers["ground"], fov, angle, pos, near, far, vp);

  for (auto o : objects) {
    vec2 vo = (o.pos - pos);
    vo.normalize();
    vec2 forward(0, -1);
    forward *= mat3::rotation(angle);

    // TODO: provide a "is_point_in_frustrum" check
    if(forward.dot(vo) > 0) { // check if object is in front of us
      vec2 vs = world_to_screen(o.pos, fov, angle, pos, near, far, vp);
      float dist = (o.pos - pos).length();
      int alpha = ((500 - dist) / 500) * 255;
      fb.alpha = alpha;
      rect sr(120, 112, 8, 16);
      fb.blit(sprites, sr, vs - point(4, 15));
    }
  }
  uint32_t ms_end = now();
  /*
  fb.alpha = 255;
  fb.pen(rgba(255, 255, 255));
  fb.text("N: " + std::to_string(int(near)) + " - F:" + std::to_string(int(far)), &minimal_font[0][0], rect(0, 0, 100, 10));
  fb.text("X: " + std::to_string(int(pos.x)) + " - Y:" + std::to_string(int(pos.y)), &minimal_font[0][0], rect(0, 10, 100, 10));
  fb.text("A: " + std::to_string(int(rad2deg(angle))), &minimal_font[0][0], rect(0, 20, 100, 10));
  */
  // draw the mini map
  /*
  fb.pen(rgba(0, 0, 0, 100));
  fb.rectangle(rect(160 - 64, 120 - 64, 64, 64));
  vec2 mmp;
  for (mmp.y = 0; mmp.y < 64; mmp.y++) {
    for (mmp.x = 0; mmp.x < 64; mmp.x++) {
      point tp = mmp * 2.0f;
      
      int16_t tile_id = map.layers["ground"].tile_at(tp) - 1;

      if (tile_id != -1) {
        point sp(
          (tile_id & 0b1111),
          (tile_id / 16)
        ); // sprite sheet coordinates

        rgba *spr = (rgba *)sprites.mipmaps[2].data;
        rgba *mmv = &spr[sp.x + sp.y * 16];
        fb.pen(*mmv);
        fb.pixel(mmp + vec2(160 - 64, 120 - 64));
      }
    }
  }*/
  
  vec2 mmpos = (pos / 16.0f) + vec2(160 - 64, 120 - 64);
  fb.pen(rgba(255, 255, 255));
  fb.pixel(mmpos);

  vec2 mtpos = (vec2(400, 400) / 16.0f) + vec2(160 - 64, 120 - 64);
  fb.pen(rgba(0, 255, 255));
  fb.pixel(mtpos);

  vec2 forward(0, -10);
  forward *= mat3::rotation(angle);
  fb.pen(rgba(255, 255, 255, 100));
  fb.line(mmpos, mmpos + forward);

  // draw FPS meter
  fb.alpha = 255;
  fb.pen(rgba(255, 255, 255, 100));
  fb.rectangle(rect(1, 120 - 10, 12, 9));
  fb.pen(rgba(255, 255, 255, 200));
  std::string fms = std::to_string(ms_end - ms_start);
  fb.text(fms, &minimal_font[0][0], rect(3, 120 - 9, 10, 16));

  int block_size = 4;
  for (int i = 0; i < (ms_end - ms_start); i++) {
    fb.pen(rgba(i * 5, 255 - (i * 5), 0));
    fb.rectangle(rect(i * (block_size + 1) + 1 + 13, fb.bounds.h - block_size - 1, block_size, block_size));
  }
}

void update(uint32_t time) {
  static float angle_delta = 0.0f;

  if (pressed(button::DPAD_LEFT))  { angle_delta += 0.1f; }
  if (pressed(button::DPAD_RIGHT)) { angle_delta -= 0.1f; }
  if (pressed(button::DPAD_UP))    { vel.y += 0.2f; }
  if (pressed(button::DPAD_DOWN))  { vel.y -= 0.2f; }

  angle += deg2rad(angle_delta);   
  mat3 r = mat3::rotation(angle);    
  pos = pos - (vel * r);

  if (pressed(button::A)) {
    far = far + (far * 0.025f);
  }
  if (pressed(button::B)) {
    far = far - (far * 0.025f);
  }

  if (pressed(button::X)) {
    fov = fov + (fov * 0.025f);
  }
  if (pressed(button::Y)) {
    fov = fov - (fov * 0.025f);
  }

  angle_delta *= 0.95f;
  vel *= 0.95f;

}
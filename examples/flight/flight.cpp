
#include <string>
#include <cstring>
#include <memory>
#include <cstdlib>

#include "graphics/mode7.hpp"

#include "flight.hpp"
#include "assets.hpp"

using namespace blit;

const uint16_t screen_width = 160;
const uint16_t screen_height = 120;

// extra space allocated to take mipmaps
// mipmaps are stored as Pen since they're the blended result of scaling the paletted image data
uint8_t __sprites[(128 * 128) + (64 * 128 * sizeof(Pen))] __SECTION__(".ss");

// storage for the water spritesheet
uint8_t __water[64 * 64] __SECTION__(".ss");

/* create surfaces */
Surface *sprites;
Surface *water;

Map map(Rect(0, 0, 128, 128));

struct object {
  Vec2 pos;
  uint8_t type;

  object(Vec2 pos, uint8_t type) : pos(pos), type(type) {}
};

struct DrawObject {
  object o;
  float dist;
  Vec2 vs;
  
  DrawObject(object obj, float dist, Vec2 vs): o(obj), dist(dist), vs(vs) {}

  bool operator< (const DrawObject &other) const {
        return other.dist < dist;
    }
};

std::vector<object> objects;

static Vec2 vel(0, 0);
static float angle = -15.0f * (pi / 180.0f);
Vec2 pos(512, 512);

float map_size = 128;
float fov = 95.0f * (pi / 180.0f);
float far = 500.0f;
float near = 10.0f;
float is_starting = false;

float deg2rad(float a) {
  return a * (pi / 180.0f);
}

float rad2deg(float r) {
  return r * (180.0f / pi);
}

void init() {
  map.add_layer("ground", layer);
  map.layers["ground"].transforms = layer_transforms;

  // Load our map sprites into the __sprites space we've reserved
  sprites = Surface::load(packed_data, __sprites);
  sprites->generate_mipmaps(3);

  water = Surface::load(water_packed_data, __water);

  // extract information about objects from the map data
  Point p;
  for (p.y = 0; p.y < 128; p.y++) {
    for (p.x = 0; p.x < 128; p.x++) {
      int16_t tid = map.layers["ground"].tile_at(p);
      if (tid == 27) {
        objects.emplace_back(
          Vec2(p.x * 8 + 4, p.y * 8 + 4),
          1
        );
      }
      if (tid == 28) {
        objects.emplace_back(
          Vec2(p.x * 8 + 4, p.y * 8 + 4),
          2
        );
      }
      if (tid == 29) {
        objects.emplace_back(
          Vec2(p.x * 8 + 4, p.y * 8 + 4),
          3
        );
      }
      if (tid == 30) {
        objects.emplace_back(
          Vec2(p.x * 8 + 4, p.y * 8 + 4),
          4
        );
      }
    }
  }
}

/* get normalised value off 255 for the dist of object between the above values. 
  So if min is 300 and max is 800, if the distance is 400, we would want to get a scale of the alpha
*/
float calulateFadeAlpha (const int maxDist, const int minDist, float dist) {
  int range = maxDist - minDist;
  int n = dist - minDist;
  float percentage = n / float(range);
  return 1.0f - (255.0f * percentage);
}

Rect vp(0, 50, 160, 120 - 50);

// returns vector of objects that are viewable
std::vector<DrawObject> drawObjects (std::vector<object> objects) {
  std::vector<DrawObject> vect;

  for (auto o : objects) {
    Vec2 vo = (o.pos - pos);
    vo.normalize();
    Vec2 forward(0, -1);
    forward *= Mat3::rotation(angle);

    // TODO: provide a "is_point_in_frustrum" check
    if(forward.dot(vo) > 0) { // check if object is in front of us
      Vec2 vs = world_to_screen(o.pos, fov, angle, pos, near, far, vp);
      float dist = (o.pos - pos).length();
      
      vect.push_back(DrawObject(o,dist,vs));
    }
  }

  return vect;
}

void render(uint32_t time_ms) {
  static int tick_count = 0; tick_count++;

  screen.alpha = 255;
  screen.mask = nullptr;
  screen.pen = Pen(99, 155, 255);
  screen.rectangle(Rect(0, 0, 160, 50));

  screen.pen = Pen(91, 110, 225);
  screen.rectangle(Rect(0, 50, 160, 120 - 50));
  uint32_t ms_start = now();

  screen.alpha = 55;

  screen.blit(water, Rect(0, 0, 64, 64), Point(0, 50));

  mode7(&screen, sprites, &map.layers["ground"], fov, angle, pos, near, far, vp);

  std::vector<DrawObject> drawables = drawObjects(objects);
  std::sort(drawables.begin(), drawables.end()); // sort them so they draw in order

  const int maxViewDistance = 600; // draw distance
  const int maxClearDistance = 300; // before sprites stop being fully opaque. start to fade out.

  for (DrawObject o : drawables) {
   
    if (o.dist < float(maxViewDistance)) { // check if the object is in a reasonable distance

      if (o.dist > maxClearDistance) {
        screen.alpha = calulateFadeAlpha(maxViewDistance, maxClearDistance, o.dist); 
      } else {
        screen.alpha = 255.0f;
      }

      Rect sr(120, 112, 8, 16);

      screen.blit(sprites, sr, o.vs - Point(4, 15));
    }
  }

  uint32_t ms_end = now();

  // Orientation debug info
  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.text("N: " + std::to_string(int(near)) + " - F:" + std::to_string(int(far)), minimal_font, Rect(0, 0, 100, 10));
  screen.text("X: " + std::to_string(int(pos.x)) + " - Y:" + std::to_string(int(pos.y)), minimal_font, Rect(0, 10, 100, 10));
  screen.text("A: " + std::to_string(int(rad2deg(angle))), minimal_font, Rect(0, 20, 100, 10));

  // draw the mini map
  screen.alpha = 200;
  screen.pen = Pen(0, 0, 0, 100);
  screen.rectangle(Rect(160 - 64, 120 - 64, 64, 64));
  Vec2 mmp;
  for (mmp.y = 0; mmp.y < 64; mmp.y++) {
    for (mmp.x = 0; mmp.x < 64; mmp.x++) {
      Point tp = mmp * 2.0f;
      
      int16_t tile_id = map.layers["ground"].tile_at(tp) - 1;

      if (tile_id != -1) {
        Point sp(
          (tile_id & 0b1111),
          (tile_id / 16)
        ); // sprite sheet coordinates

        Pen *spr = (Pen *)sprites->mipmaps[2]->data;
        Pen *mmv = &spr[sp.x + sp.y * 16];
        screen.pen = *mmv;
        screen.pixel(mmp + Vec2(160 - 64, 120 - 64));
      }
    }
  }
  
  Vec2 mmpos = (pos / 16.0f) + Vec2(160 - 64, 120 - 64);
  screen.pen = Pen(255, 255, 255);
  screen.pixel(mmpos);

  Vec2 mtpos = (Vec2(400, 400) / 16.0f) + Vec2(160 - 64, 120 - 64);
  screen.pen = Pen(0, 255, 255);
  screen.pixel(mtpos);

  Vec2 forward(0, -10);
  forward *= Mat3::rotation(angle);
  screen.pen = Pen(255, 255, 255, 100);
  screen.line(mmpos, mmpos + forward);

  // draw FPS meter
  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255, 100);
  screen.rectangle(Rect(1, 120 - 10, 12, 9));
  screen.pen = Pen(255, 255, 255, 200);
  std::string fms = std::to_string(ms_end - ms_start);
  screen.text(fms, minimal_font, Rect(3, 120 - 9, 10, 16));

  int block_size = 4;
  for (uint32_t i = 0; i < (ms_end - ms_start); i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(Rect(i * (block_size + 1) + 1 + 13, screen.bounds.h - block_size - 1, block_size, block_size));
  }
}

bool compare (float v1, float v2) {
  return fabsf(v1 - v2) < 0.001f;
}

bool is_off_ground () {
  return !compare(500.0f, far);
}

float lerping(float a, float b, float f) {
    return a + f * (b - a);
}

void update(uint32_t time) {
  
  static float angle_delta = 0.0f;
  float target_speed;
  float lerp_value = 0.002f;

  if (pressed(Button::DPAD_LEFT))  { angle_delta += 0.05f; }
  if (pressed(Button::DPAD_RIGHT)) { angle_delta -= 0.05f; }

  angle_delta -= joystick.x / 80.0f ;

  if (pressed(Button::Y)) { 
    // boost button
      target_speed = is_off_ground() ? 2.0f : 0.5f;
  } else {
      target_speed = is_off_ground() ? 0.8f : 0.0f;
  }

  if (pressed(Button::X))  { 
    // break button
    target_speed = is_off_ground() ? 0.5f : 0.0f;

    if (!is_off_ground()) {
      lerp_value *= 5.0f;
    }
  }

  angle += deg2rad(angle_delta);   
  Mat3 r = Mat3::rotation(angle);    
  pos = pos - (vel * r);

  if (pressed(Button::A) || (joystick.y > 0.0f)) {
    far = far + (far * 0.01f);
  }
  if (pressed(Button::B) || (joystick.y < 0.0f)) {
    far = far - (far * 0.01f);
    far = std::max(far,500.0f);
  }

  angle_delta *= 0.95f;

  vel.y = lerping(vel.y, target_speed, 0.002f);

}
#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "tilemap_test.hpp"

using namespace blit;
  
static uint8_t layer_world[] = { 
  57, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
  57, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
  57, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
  57, 0, 51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
  57, 0, 67, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
  57, 0, 83, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
  70, 57, 57, 57, 57, 57, 57, 57, 57, 56, 55, 56, 57, 57, 57, 57,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 0, 57, 0, 0, 0, 0,
};

static uint8_t layer_world_transforms[] = {
  5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  3, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0, 0,
};

std::map<int, int> indexes = {
  {70, 0},
  {56, 1},
  {57, 2},
  {0,  3},
  {13, 4},
  {67, 5},
  {55, 6},
  {7,  7},
  {83, 8}
};

// TODO: tilemap needs an overloaded constructor to avoid the need to pass nullptr for sprites
TileMap world((uint8_t *)layer_world, nullptr, Size(16, 8), nullptr);

/* setup */
void init() {
  blit::set_screen_mode(ScreenMode::hires);

  // fix tile ids from .tmx file
/*  for (auto i = 0; i < (64 * 64); i++) {
    if(layer_background[i] != 0)
      layer_background[i]--;
    if (layer_environment[i] != 0)
      layer_environment[i]--;
  }*/

  //sprites.s.generate_mipmaps(3);
  screen.sprites = SpriteSheet::load(packed_data);

  world.transforms = layer_world_transforms;
  // TOTO: tilemap could use a public API interface to set_sprites
  world.sprites = screen.sprites;
}

Mat3 world_transform;
 

float deg2rad(float a) {
  return a * (M_PI / 180.0f);
}

uint32_t current_time;

std::function<Mat3(uint8_t)> dream = [](uint8_t y) -> Mat3 {
  float progress = (sin(current_time / 2000.0f) + 1.0f) * 0.6f;  // two second animation
  progress = progress > 1.0f ? 1.0f : progress;

  screen.alpha = progress * 255.0f;
  float step = (current_time / 200.0f) + (y / 10.0f);
  float x_offset = (sin(step) * (cos(step) * 50.0f)) * (1.0f - progress);

  Mat3 transform = Mat3::identity();
  transform *= Mat3::translation(Vec2(256, 156)); // offset to middle of world      
  transform *= Mat3::translation(Vec2(x_offset, 0)); // apply dream effect wave      
  transform *= Mat3::translation(Vec2(-80, -60)); // transform to centre of framebuffer

  return transform;
};

std::function<Mat3(uint8_t)> rotozoom = [](uint8_t y) -> Mat3 {
  float progress = (sin(current_time / 2000.0f) + 1.0f) / 2.0f;  // two second animation

  float angle = (1.0f - progress) * 360.0f;
  float scale = progress;

  Mat3 transform = Mat3::identity();
  transform *= Mat3::translation(Vec2(256, 156)); // offset to middle of world      
  transform *= Mat3::rotation(deg2rad(angle)); // apply dream effect wave      
  transform *= Mat3::scale(Vec2(scale, scale)); // apply dream effect wave      
  transform *= Mat3::translation(Vec2(-80, -60)); // transform to centre of framebuffer

  return transform;
};

std::function<Mat3(uint8_t)> zoom = [](uint8_t y) -> Mat3 {
  float progress = (sin(current_time / 1000.0f) + 1.0f) / 2.0f;  // two second animation    
    
  float scale = progress;

  Mat3 transform = Mat3::identity();
  transform *= Mat3::translation(Vec2(256, 156)); // offset to middle of world      
  transform *= Mat3::scale(Vec2(scale, scale)); // apply dream effect wave      
  transform *= Mat3::translation(Vec2(-80, -60)); // transform to centre of framebuffer

  return transform;
};


std::function<Mat3(uint8_t)> perspective = [](uint8_t y) -> Mat3 {
  float progress = (sin(current_time / 2000.0f) + 1.0f) / 2.0f;  // two second animation
  progress = progress > 1.0f ? 1.0f : progress;

  screen.alpha = y + (255 - 120);
  float scale = y / 30.0f;

  Mat3 transform = Mat3::identity();
  transform *= Mat3::translation(Vec2(156, 130 + progress * 80.0f)); // offset to middle of world      
  transform *= Mat3::scale(Vec2(1.0f / scale, 1.0f / scale)); // apply dream effect wave      
  transform *= Mat3::translation(Vec2(-80, -60)); // transform to centre of framebuffer

  return transform;
};


std::function<Mat3(uint8_t)> water = [](uint8_t y) -> Mat3 {
  float step = (current_time / 200.0f) + (y / 10.0f);
  float x_offset = (sin(step) + sin(step / 3.0f) + sin(step * 2.0f)) * 2.0f;

  Mat3 transform = Mat3::identity();
  transform *= Mat3::translation(Vec2(156, 130)); // offset to middle of world      
  transform *= Mat3::translation(Vec2(x_offset, 0)); // apply dream effect wave      
  transform *= Mat3::translation(Vec2(-80, -60)); // transform to centre of framebuffer

  return transform;
};

std::function<Mat3(uint8_t)> warp = [](uint8_t y) -> Mat3 {
  float step = (current_time / 1000.0f) + (y / 100.0f);
  float x_offset = (sin(step) * (cos(step) * 50.0f));

  float angle = sin(current_time / 500.0f) * 50.0f;

  Mat3 transform = Mat3::identity();
  transform *= Mat3::translation(Vec2(50, 30)); // offset to middle of world      
  transform *= Mat3::translation(Vec2(x_offset, 0)); // apply dream effect wave      
  transform *= Mat3::rotation(deg2rad(angle)); // apply dream effect wave      
  transform *= Mat3::translation(Vec2(-80, -60)); // transform to centre of framebuffer

  return transform;
};

std::function<Mat3(uint8_t)> ripple = [](uint8_t y) -> Mat3 {
  float step = (current_time / 250.0f) + (y / 25.0f);
    
  float scale = (sin(step) / 4.0f) + 1.0f;

  Mat3 transform = Mat3::identity();
  transform *= Mat3::translation(Vec2(256, 156)); // offset to middle of world      
  transform *= Mat3::scale(Vec2(scale, scale)); // apply dream effect wave      
  transform *= Mat3::translation(Vec2(-80, -60)); // transform to centre of framebuffer

  return transform;
};

std::function<Mat3(uint8_t)> effect_callbacks[]{
  zoom,
  dream,
  ripple,
  rotozoom,
  warp,
  perspective,
  water
};

std::vector<std::string> effect_names = {
  "zoom",
  "dream",
  "ripple",
  "rotozoom",
  "warp",
  "perspective",
  "water"
};

uint8_t effect = 0;

void render(uint32_t time_ms) {
  current_time = time_ms;

  screen.alpha = 255;
  screen.pen = Pen(0, 0, 0);
  screen.clear();

  screen.alpha = 255;
  screen.mask = nullptr;

  uint32_t ms_start = now();
  

  int highlight = (time_ms / 1000) % 9;
  uint8_t fade = ((sin(time_ms / 1000.0f) + 1.1f) / 2.2f) * 254.0f;

  //world.repeat_mode = tilemap::REPEAT;
  Vec2 wo(64, 40);

  world.transform =
    Mat3::identity() *
    Mat3::translation(wo) *
    Mat3::scale(Vec2(0.5, 0.5)) *
    Mat3::translation(Vec2(-160, -120));
  Point tl = Point(160, 120) - (wo * 2.0f);

  world.draw(&screen, Rect(0, 0, 320, 240), nullptr);
  /*

  screen.alpha = fade;
  screen.pen = rgba(0, 0, 0);
  screen.clear();*/

  screen.alpha = 255;
  int highlight_sprite = 0;
  for (std::map<int, int>::iterator iter = indexes.begin(); iter != indexes.end(); ++iter)
  {
    if (iter->second == highlight)
      highlight_sprite = iter->first;
  }
  screen.pen = Pen(0, 255, 0, 100);
  for (int y = 0; y < world.bounds.h; y++) {
    for (int x = 0; x < world.bounds.w; x++) {
      if (highlight_sprite == world.tile_at(Point(x, y))) {
        screen.rectangle(Rect(tl.x + (x * 16), tl.y + (y * 16), 17, 17));
      }
    }
  }


  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  for (int y = 0; y < world.bounds.h; y++) {
    for (int x = 0; x < world.bounds.w; x++) {
      std::string t = std::to_string(indexes[layer_world[world.offset(x, y)]]);
      screen.text(t, &minimal_font[0][0], tl + Point(x * 8 * 2 + 7, y * 8 * 2 + 5));
    }
  }


  // draw grid
  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));
  screen.pen = Pen(0, 0, 0);
  screen.text("Tilemap sprite indexes", &minimal_font[0][0], Point(5, 4));

  screen.pen = Pen(255, 255, 255);
  screen.text("Tilemap:", &minimal_font[0][0], Point(5, 25));
  /*
  screen.pen = rgba(255, 255, 255);
  for (int y = 0; y < world.bounds.h; y++) {    
    screen.text(std::to_string(y), &minimal_font[0][0], tl + point(-8, y * 8 * 2 + 5));
  }*/

  screen.pen = Pen(255, 255, 255, 100);
  for (int y = 0; y <= world.bounds.h; y++) {    
    screen.line(tl + Point(0, y * 8 * 2), tl + Point(128 * 2, y * 8 * 2));
  }
  /*
  screen.pen = rgba(255, 255, 255);
  for (int x = 0; x < world.bounds.w; x++) {    
    screen.text(std::to_string(x), &minimal_font[0][0], tl + point(x * 8 * 2 + 9 - std::to_string(x).length() * 2, -8));
  }*/

  screen.pen = Pen(255, 255, 255, 100);
  for (int x = 0; x <= world.bounds.w; x++) {    
    screen.line(tl + Point(x * 8 * 2, 0), tl + Point(x * 8 * 2, 64 * 2));
  }  

  screen.pen = Pen(255, 255, 255);
  screen.text("Spritesheet:", &minimal_font[0][0], Point(5, 180));

  for (int i = 0; i < 9; i++) {
    int found = 0;
    for (std::map<int, int>::iterator iter = indexes.begin(); iter != indexes.end(); ++iter)
    {
      if (iter->second == i)
        found = iter->first;
    }

    Rect sb = screen.sprites->sprite_bounds(found);


    screen.pen = Pen(255, 255, 255);
    screen.text(std::to_string(i), &minimal_font[0][0], Point(32 + 56 + i * 16 + 6, 195));

    screen.stretch_blit(screen.sprites, sb, Rect(32 + 56 + i * 16, 205, 16, 16));
    //sprites.draw(&fb, point(10 + i * 8, 200), found, false);
  }

  screen.pen = Pen(0, 255, 0, 100);
  screen.rectangle(Rect(32 + 56 + highlight * 16, 205, 16, 16));

  screen.pen = Pen(255, 255, 255, 100);
  screen.line(Point(32 + 56, 205), Point(32 + 56 + 9 * 16, 205));
  screen.line(Point(32 + 56, 205 + 16), Point(32 + 56 + 9 * 16, 205 + 16));
  for (int i = 0; i <= 9; i++) {
    screen.line(Point(32 + 56 + i * 16, 205), Point(32 + 56 + i * 16, 205 + 16));
  }

  /*
  for(int i = 0; i < 100; i++)
    world.draw(&fb, rect(0, 0, 160, 120), effect_callbacks[effect]);
    */
    /*
  screen.alpha = 255;
  for (auto i = 0; i < effect_names.size(); i++) {
    if(effect == i)
      screen.pen = rgba(255, 255, 255);
    else
      screen.pen = rgba(255, 255, 255, 100);

    screen.text(effect_names[i], &minimal_font[0][0], rect(2, 2 + (i * 10), 100, 10));      
  }

  uint32_t ms_end = now();
    
  // draw FPS meter
  screen.alpha = 255;
  screen.pen = rgba(0, 0, 0);
  screen.rectangle(rect(1, 120 - 10, 12, 9));
  screen.pen = rgba(255, 255, 255, 200);
  std::string fms = std::to_string(ms_end - ms_start);
  screen.text(fms, &minimal_font[0][0], rect(3, 120 - 9, 10, 16));

  int block_size = 4;
  for (int i = 0; i < (ms_end - ms_start); i++) {
    screen.pen = rgba(i * 5, 255 - (i * 5), 0);
    screen.rectangle(rect(i * (block_size + 1) + 1 + 13, screen.bounds.h - block_size - 1, block_size, block_size));
  }*/
}


uint32_t last_buttons = 0;
void update(uint32_t current_time) {           
  if (buttons != last_buttons) {
    if (pressed(Button::DPAD_DOWN)) {
      effect = effect == effect_names.size() - 1 ? 0 : effect + 1;
    }
    if (pressed(Button::DPAD_UP)) {
      effect = effect == 0 ? effect_names.size() - 1 : effect - 1;
    }
  }

    
  last_buttons = buttons;
}

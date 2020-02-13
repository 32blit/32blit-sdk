#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "tilemap-test.hpp"

using namespace blit;


  TileMap* background;
  TileMap* environment;

  /* setup */
  void init() {
    blit::set_screen_mode(ScreenMode::lores);

    // fix tile ids from .tmx file
    for (auto i = 0; i < (64 * 64); i++) {
      if (layer_environment[i] != 0)
        layer_environment[i]--;
    }

    //sprites.generate_mipmaps(3);
    
    screen.sprites = SpriteSheet::load(packed_data);
    environment = new TileMap((uint8_t*)layer_environment, nullptr, Size(64, 64), screen.sprites);
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
    transform *= Mat3::translation(Vec2(140, 140)); // offset to middle of world      
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

  std::function<Mat3(uint8_t)> betamax = [](uint8_t y) -> Mat3 {
    float step = (current_time / 250.0f) + (y / 25.0f);

    int8_t scale = int8_t((sin(step) + 1.0f) * 5);
    Point shake((blit::random() % scale) - scale / 2, (blit::random() % scale) - scale / 2);

    Mat3 transform = Mat3::identity();
    transform *= Mat3::translation(Vec2(256 + shake.x, 156 + shake.y)); // offset to middle of world      
    transform *= Mat3::translation(Vec2(-80, -60)); // transform to centre of framebuffer

    return transform;
  };

  std::function<Mat3(uint8_t)> shake = [](uint8_t y) -> Mat3 {
    static uint32_t last_time = 0;
    static Point shake(0, 0);

    if (current_time != last_time) {
      last_time = current_time;

      uint8_t clamp = 20;
      shake = Point(blit::random() % clamp, blit::random() % clamp);
      float scale = sin(current_time / 300.0f);

      scale = scale < 0.0f ? 0.0f : scale;
      shake *= scale;
    }
   
    Mat3 transform = Mat3::identity();
    transform *= Mat3::translation(Vec2(256 + shake.x, 156 + shake.y)); // offset to middle of world      
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
    water,
    betamax,
    shake
  };

  std::vector<std::string> effect_names = {
    "zoom",
    "dream",
    "ripple",
    "rotozoom",
    "warp",
    "perspective",
    "water",
    "betamax",
    "shake"
  };

  uint8_t effect = 0;

  void render(uint32_t time_ms) {
    current_time = time_ms;

    screen.alpha = 255;
    screen.pen = Pen(39, 39, 54);
    screen.clear();

    screen.alpha = 255;
    screen.mask = nullptr;

    uint32_t ms_start = now();

    environment->draw(&screen, Rect(0, 0, 160, 120), effect_callbacks[effect]);


    screen.alpha = 255;
    for (auto i = 0; i < effect_names.size(); i++) {
      if (effect == i)
        screen.pen = Pen(255, 255, 255);
      else
        screen.pen = Pen(255, 255, 255, 100);

      screen.text(effect_names[i], minimal_font, Rect(2, 2 + (i * 10), 100, 10));
    }

    uint32_t ms_end = now();
    /*
    // draw FPS meter
    screen.alpha = 255;
    screen.pen(Pen(0, 0, 0));
    screen.rectangle(rect(1, 120 - 10, 12, 9));
    screen.pen(Pen(255, 255, 255, 200));
    std::string fms = std::to_string(ms_end - ms_start);
    screen.text(fms, minimal_font, rect(3, 120 - 9, 10, 16));

    int block_size = 4;
    for (int i = 0; i < (ms_end - ms_start); i++) {
      screen.pen(Pen(i * 5, 255 - (i * 5), 0));
      screen.rectangle(rect(i * (block_size + 1) + 1 + 13, screen.bounds.h - block_size - 1, block_size, block_size));
    }*/
  }


  uint32_t last_buttons = 0;
  void update(uint32_t time) {
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
#include <string>
#include <cstring>
#include <memory>
#include <cstdlib>

#include "sprite-test.hpp"
#include "assets.hpp"

using namespace blit;

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;

/* setup */
void init() {
  // You gotta load those tasty sprites first
  // "asset_dingbads" is the asset name defined in assets.yml
  screen.sprites = Surface::load(asset_dingbads);
}

int tick_count = 0;
void render(uint32_t time_ms) {
  screen.pen = Pen(20, 30, 40);
  screen.clear();

  screen.alpha = 255;
  screen.mask = nullptr;

  // draw grid
  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));

  // Left Titles
  screen.text("Apple", minimal_font, Point(5, 20));
  screen.text("Skull", minimal_font, Point(5, 40));
  screen.text("Flowers", minimal_font, Point(5, 60));
  screen.text("Rotate", minimal_font, Point(5, 80));
  screen.text("Flip", minimal_font, Point(5, 100));

  // Right Titles
  screen.text("Big\n(Blit)", minimal_font, Point(85, 20));
  screen.text("Big\n(Sprite)", minimal_font, Point(85, 40));

  screen.pen = Pen(0, 0, 0);
  screen.text("Sprite demo", minimal_font, Point(5, 4));

  uint32_t ms_start = now();

  // Left Examples

  // Draw a sprite using its numerical index into the sprite sheet
  // Treats the sprite sheet as a grid of 8x8 sprites numbered from 0 to 63
  // In this case sprite number 1 is the second sprite from the top row.
  // It should be an apple! Munch!
  screen.sprite(1, Point(50, 20));

  // Draw a sprite using its X/Y position from the sprite sheet
  // Treats the sprite sheet as a grid of 8x8 sprites
  // numbered 0 to 15 across and 0 to 15 down!
  // In this case we draw the sprite from:
  // The 10th position across (0 based, remember!)
  // The 3rd position down.
  // It should be a skull! Yarr!
  screen.sprite(Point(9, 2), Point(50, 40));

  // Draw a group of sprites starting from an X/Y position, with a width/height
  // Treats the sprite sheet a grid of 8x8 sprites and selects a group of them defined by a Rect(x, y, w, h)
  // The width and height are measured in sprites.
  // In this case we draw three sprites from the 6th column on the 12th row.
  // It should be a row of flowers! Awww!
  screen.sprite(Rect(5, 11, 3, 1), Point(50, 60));

  // Draw a heart rotated 90, 180 and 270 degrees
  screen.pen = Pen(40, 60, 80);
  screen.rectangle(Rect(50, 80, 8, 8));
  screen.sprite(Point(0, 4), Point(50, 80), SpriteTransform::R90);
  screen.rectangle(Rect(60, 80, 8, 8));
  screen.sprite(Point(0, 4), Point(60, 80), SpriteTransform::R180);
  screen.rectangle(Rect(70, 80, 8, 8));
  screen.sprite(Point(0, 4), Point(70, 80), SpriteTransform::R270);

  // Draw a heart flipped horiontally and vertically
  screen.rectangle(Rect(50, 100, 8, 8));
  screen.sprite(Point(0, 4), Point(50, 100), SpriteTransform::HORIZONTAL);
  screen.rectangle(Rect(60, 100, 8, 8));
  screen.sprite(Point(0, 4), Point(60, 100), SpriteTransform::VERTICAL);

  // Right examples

  // Draw a cherry, stretched to 16x16 pixels
  screen.stretch_blit(
    screen.sprites,
    Rect(0, 0, 8, 8),
    Rect(130, 16, 16, 16)
  );

  // Also, stretched with the scale factor of the normal sprite functions
  // The second Point() argument here is the origin for any transform applied,
  // but here we aren't applying one.
  screen.sprite(0, Point(130, 40), Point(0, 0), 2.0f);

  uint32_t ms_end = now();


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

  screen.watermark();
}

void update(uint32_t time) {

}

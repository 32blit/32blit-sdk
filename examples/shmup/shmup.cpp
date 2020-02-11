#include "shmup.hpp"

using namespace blit;

#define SCENERY_BIG_SKULL Rect(11, 3, 4, 3)
#define SCENERY_TUSK Rect(10, 3, 1, 3)
#define SCENERY_BIG_TUSK Rect(10, 6, 2, 3)
#define SCENERY_BIG_BONE Rect(12, 6, 3, 2)
#define SCENERY_CLAWS Rect(6, 5, 3, 2)
#define SCENERY_BIG_ROCK Rect(3, 5, 3, 3)

#define SHOT_BIG_RED Rect(6, 0, 2, 1)
#define SHOT_LONG_RED Rect(6, 0, 4, 1)
#define SHOT_BIG_PINK Rect(10, 0, 2, 1)

// These lookup tables include an offset in pixels
// as the first two uint8_t values.
// We offset in pixels rather than tiles to avoid having to
// multiply the values up when the sprite is drawn.
// The following values are bitmasks describing a sprite with
// a maximum of 8 tiles width, and unlimited height.
// Each bit, from left to right, indicates a tile starting from
// the offset that should be included in this sprite.
// This is much more space efficient than storing a point(x, y)
// for each tile we want in a sprite.

// Weird ship with a hole in it
const std::vector<uint8_t> BG_SHIP_1 = { \
                    12 * 8, 5 * 8, \
                    4, \
                    0b00100000, \
                    0b01110000, \
                    0b11110000 };

// Background ship with all the shuttle bays and little winglets on the bottom
const std::vector<uint8_t> BG_SHIP_2{ \
                    9 * 8, 4 * 8, \
                    5, \
                    0b11111000, \
                    0b11111000, \
                    0b01110000 };

// Hog ship!
const std::vector<uint8_t> HOG_SHIP{ \
                    10 * 8, 0, \
                    6, \
                    0b11111100, \
                    0b11111100, \
                    0b11111100, \
                    0b11111100 };

/*
Map

  [] --> [] --> [] --> []
      \
       [] --> [] --> []

Player
  x, y
  vx, vy
  life
Enemy
  x, y
  hit
  life
Projectile
  x, y
  vx, vy
Background Scenery
  x, y

*/
bool hit = false;

Pen hit_palette[2][5] = {
  {},
  {}
};

Size boar_ship_size(32, 32);

SpriteSheet* ships;
SpriteSheet* background;

Tween tween_bob;
Tween tween_dusk_dawn;
Tween tween_parallax;

void draw_tilebased_sprite(SpriteSheet* ss, Point origin, const std::vector<uint8_t> ship, bool hflip = false) {
  // We can use uint8_t everywhere here, but this limits us
  // to a spritesheet of 256*256. That's fine!
  uint8_t o_x = ship[0];
  uint8_t o_y = ship[1];

  // We need to know the width of the ship within our bitmask
  // so we can horizontally flip if needed without it going wonky
  uint8_t w = ship[2];

  for (auto i = 0; i < (uint8_t)ship.size() - 3; i++) {
    uint8_t ship_mask = ship[i + 3]; // Bitmask for tiles in a horizontal row
    uint8_t offset_y = (i * 8);
    uint8_t src_y = o_y + offset_y;
    for (auto j = 0; j < w; j++) {
      uint8_t jj = j;
      if (hflip) {
        jj = w - 1 - jj;
      }
      if (ship_mask & (0b10000000 >> jj)) {
        uint8_t offset_x = (jj * 8);
        uint8_t src_x = o_x + offset_x;
        screen.blit(ss, Rect(src_x, src_y, 8, 8), Point(origin.x + (j * 8), origin.y + offset_y), hflip);
      }
    }
  }
}

void init() {
  set_screen_mode(ScreenMode::hires);

  ships = SpriteSheet::load(sprites_ships);
  background = SpriteSheet::load(sprites_background);
  tween_bob.init(tween_sine, 0.0f, 1.0f, 2500, -1);
  tween_bob.start();

  tween_dusk_dawn.init(tween_sine, 0.0f, 1.0f, 500, 1);
  tween_dusk_dawn.start();

  tween_parallax.init(tween_linear, 0.0f, 1.0f, 10000, -1);
  tween_parallax.start();
  screen.sprites = background;

  for (int x = 0; x < 5; x++) {
    Pen s_c = screen.sprites->palette[4 + x];
    Pen c = Pen(
      std::min(255, int(s_c.r * 1.8)),
      std::min(255, int(s_c.g * 1.8)),
      std::min(255, int(s_c.b * 1.8))
    );
    hit_palette[0][x] = s_c;
    hit_palette[1][x] = c;
  }

  for (int x = 0; x < 32; x++) {
    background->palette[x] = desert[x];
  }
}

void render(uint32_t time) {
  uint32_t ms_start = blit::now() - time;
  screen.pen = background->palette[21];
  screen.clear();
  screen.sprites = background;

  for (int x = 0; x < 40; x++) {
    screen.sprite(Rect(15, 3, 1, 5), Point(x * 8, 0), 0);
    screen.sprite(Rect(0, 1, 1, 8), Point(x * 8, screen.bounds.h - 64), 0);
  }

  Pen t = background->palette[29];
  background->palette[29] = background->palette[20];

  // Far mountains
  screen.sprite(Rect(1, 2, 15, 1), Point((tween_parallax.value * 60), screen.bounds.h - 80), Point(0, 0), 2.0f, 0);
  screen.sprite(Rect(5, 1, 2, 1), Point((tween_parallax.value * 60) + (8 * 4 * 2.0f), screen.bounds.h - 96), Point(0, 0), 2.0f, 0);
  screen.sprite(Rect(12, 1, 3, 1), Point((tween_parallax.value * 60) + (8 * 11 * 2.0f), screen.bounds.h - 96), Point(0, 0), 2.0f, 0);

  // Try and palette swap in a transparent colour and draw in some reflections?
  background->palette[29] = Pen(background->palette[19].r, background->palette[19].g, background->palette[19].b, 100);
  screen.sprite(Rect(1, 2, 15, 1), Point((tween_parallax.value * 60), screen.bounds.h - 64), Point(0, 0), 2.0f, 2);
  screen.sprite(Rect(5, 1, 2, 1), Point((tween_parallax.value * 60) + (8 * 4 * 2.0f), screen.bounds.h - 48), Point(0, 0), 2.0f, 2);
  screen.sprite(Rect(12, 1, 3, 1), Point((tween_parallax.value * 60) + (8 * 11 * 2.0f), screen.bounds.h - 48), Point(0, 0), 2.0f, 2);
  background->palette[29] = t;

  screen.sprite(Rect(2, 0, 2, 1), Point(0, screen.bounds.h - 72), 0);
  screen.sprite(Rect(1, 2, 3, 1), Point(60, screen.bounds.h - 72), 0);
  screen.sprite(Rect(2, 1, 2, 1), Point(120, screen.bounds.h - 72), 0);

  background->palette[29] = background->palette[18];
  screen.sprite(Rect(2, 0, 2, 1), Point(0, screen.bounds.h - 64), 2);
  screen.sprite(Rect(1, 2, 3, 1), Point(60, screen.bounds.h - 64), 2);
  screen.sprite(Rect(2, 1, 2, 1), Point(120, screen.bounds.h - 64), 2);

  background->palette[29] = t;

  screen.sprite(SCENERY_BIG_SKULL, Point(0, screen.bounds.h - 52), 0);
  screen.sprite(SCENERY_CLAWS, Point(30, screen.bounds.h - 48), 0);
  screen.sprite(SCENERY_BIG_TUSK, Point(70, screen.bounds.h - 32), 0);
  screen.sprite(SCENERY_BIG_BONE, Point(120, screen.bounds.h - 64), 0);
  screen.sprite(SCENERY_BIG_ROCK, Point(230, screen.bounds.h - 32), 0);

  screen.sprite(SHOT_BIG_RED, Point(30, 60), 0);

  // Draw a chessboard so we can count our sprite tiles
  for (int x = 0; x < 128 / 8; x++) {
    for (int y = 0; y < 128 / 8; y++) {
      if ((y + x) & 1) {
        screen.pen = Pen(0, 0, 0, 64);
      }
      else {
        screen.pen = Pen(255, 255, 255, 64);
      }
      screen.rectangle(Rect(x * 8, y * 8, 8, 8));
    }
  }

  // Draw the whole ship spritesheet
  screen.blit(ships, Rect(0, 0, 128, 128), Point(0, 0));

  draw_tilebased_sprite(ships, Point(170, 120 + 5.0f * tween_bob.value), BG_SHIP_1);
  draw_tilebased_sprite(ships, Point(120, 120 + 10.0f * tween_bob.value), BG_SHIP_2);
  draw_tilebased_sprite(ships, Point(200, 160 + 10.0f * tween_bob.value), HOG_SHIP);
  draw_tilebased_sprite(ships, Point(60, 160 + 10.0f * tween_bob.value), HOG_SHIP, true);

  screen.alpha = 200;
  for (auto x = 0; x < 10; x++) {
    for (auto y = 0; y < 10; y++) {
      draw_tilebased_sprite(ships, Point(x * 32, y * 24 + 5.0f * tween_bob.value), BG_SHIP_1, (x + y) & 1);
    }
  }
  screen.alpha = 255;

  uint32_t ms_end = blit::now() - time;

  screen.pen = Pen(255, 100, 100);
  screen.text(std::to_string(ms_end - ms_start), &minimal_font[0][0], Point(5, 230));
}

void update(uint32_t time) {
  if (buttons & Button::A && tween_dusk_dawn.is_finished()) {
    tween_dusk_dawn.start();
  }

  for (int x = 0; x < 32; x++) {
    Pen original = Pen(
      sprites_background[x * 4 + 20],
      sprites_background[x * 4 + 21],
      sprites_background[x * 4 + 22],
      sprites_background[x * 4 + 23]
    );

    Pen d = desert[x];

    float s = 1.0f - tween_dusk_dawn.value;
    float c = tween_dusk_dawn.value;

    background->palette[x] = Pen(
      std::min(255, int((d.r * s) + (original.r * c))),
      std::min(255, int((d.g * s) + (original.g * c))),
      std::min(255, int((d.b * s) + (original.b * c))),
      original.a
    );
  }
}

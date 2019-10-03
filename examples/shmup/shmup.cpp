#include "shmup.hpp"

using namespace blit;

#define SCENERY_BIG_SKULL rect(11, 3, 4, 3)
#define SCENERY_TUSK rect(10, 3, 1, 3)
#define SCENERY_BIG_TUSK rect(10, 6, 2, 3)
#define SCENERY_BIG_BONE rect(12, 6, 3, 2)
#define SCENERY_CLAWS rect(6, 5, 3, 2)
#define SCENERY_BIG_ROCK rect(3, 5, 3, 3)

#define SHOT_BIG_RED rect(6, 0, 2, 1)
#define SHOT_LONG_RED rect(6, 0, 4, 1)
#define SHOT_BIG_PINK rect(10, 0, 2, 1)

/*
Map
    
    [] --> [] --> [] --> []

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

rgba hit_palette[2][5] = {
    {},
    {}
};

size boar_ship_size(32, 32);

spritesheet *ships;
spritesheet *background;

void init() {
  ships = spritesheet::load(sprites_background);
  background = spritesheet::load(sprites_background);

  set_screen_mode(screen_mode::hires);
  fb.sprites = background;
  
  for (int x = 0; x < 5; x++){
    rgba s_c = fb.sprites->palette[4 + x];
    rgba c = rgba(
      std::min(255, int(s_c.r * 1.8)),
      std::min(255, int(s_c.g * 1.8)),
      std::min(255, int(s_c.b * 1.8))
    );
    hit_palette[0][x] = s_c;
    hit_palette[1][x] = c;
  }

  for (int x = 0; x < 32; x++){ 
    /*background->palette[x] = rgba(
      sprites_background[x * 4 + 20],
      sprites_background[x * 4 + 21],
      sprites_background[x * 4 + 22],
      sprites_background[x * 4 + 23]
    );*/

    background->palette[x] = desert[x];
  }
}

void render(uint32_t time) {
  fb.pen(background->palette[21]);
  fb.clear();
  fb.sprites = background;

  for(int x = 0; x < 40; x++){
    fb.sprite(rect(15, 3, 1, 5), point(x * 8, 0), 0);
    fb.sprite(rect(0, 1, 1, 8), point(x * 8, fb.bounds.h - 64), 0);
  }

  rgba t = background->palette[29];
  background->palette[29] = background->palette[20];
  fb.sprite(rect(1, 2, 15, 1), point(30, fb.bounds.h - 80), point(0, 0), 2.0f, 0);
  fb.sprite(rect(5, 1, 2, 1), point(30 + (8 * 4 * 2.0f), fb.bounds.h - 96), point(0, 0), 2.0f, 0);
  fb.sprite(rect(12, 1, 3, 1), point(30 + (8 * 11 * 2.0f), fb.bounds.h - 96), point(0, 0), 2.0f, 0);
  
  /*background->palette[29] = background->palette[19];
  fb.sprite(rect(1, 2, 15, 1), point(30, fb.bounds.h - 64), point(0, 0), 2.0f, 2);
  fb.sprite(rect(5, 1, 2, 1), point(30 + (8 * 4 * 2.0f), fb.bounds.h - 48), point(0, 0), 2.0f, 2);
  fb.sprite(rect(12, 1, 3, 1), point(30 + (8 * 11 * 2.0f), fb.bounds.h - 48), point(0, 0), 2.0f, 2); */

  background->palette[29] = t;
  fb.sprite(rect(2, 0, 2, 1), point(0, fb.bounds.h - 72), 0);
  fb.sprite(rect(1, 2, 3, 1), point(60, fb.bounds.h - 72), 0);
  fb.sprite(rect(2, 1, 2, 1), point(120, fb.bounds.h - 72), 0);

  /* background->palette[29] = background->palette[18];
  fb.sprite(rect(2, 0, 2, 1), point(0, fb.bounds.h - 64), 2);
  fb.sprite(rect(1, 2, 3, 1), point(60, fb.bounds.h - 64), 2);
  fb.sprite(rect(2, 1, 2, 1), point(120, fb.bounds.h - 64), 2);*/

  background->palette[29] = t;

  fb.sprite(SCENERY_BIG_SKULL, point(0, fb.bounds.h - 52), 0);
  fb.sprite(SCENERY_CLAWS, point(30, fb.bounds.h - 48), 0);
  fb.sprite(SCENERY_BIG_TUSK, point(70, fb.bounds.h - 32), 0);
  fb.sprite(SCENERY_BIG_BONE, point(120, fb.bounds.h - 64), 0);
  fb.sprite(SCENERY_BIG_ROCK, point(230, fb.bounds.h - 32), 0);

  fb.sprite(SHOT_BIG_RED, point(30, 60), 0);

  for (int x = 0; x < 32; x++){ 
    rgba original = rgba(
      sprites_background[x * 4 + 20],
      sprites_background[x * 4 + 21],
      sprites_background[x * 4 + 22],
      sprites_background[x * 4 + 23]
    );

    rgba d = desert[x];

    //float s = sin(time / 1000.0f) + 1.0f / 2.0f;
    //float c = cos(time / 1000.0f) + 1.0f / 2.0f;

    float s = time / 5000.0f;
    s -= floor(s);
    float c = 1.0f - s;

    background->palette[x] = rgba(
      std::min(255, int((d.r * s) + (original.r * c))),
      std::min(255, int((d.g * s) + (original.g * c))),
      std::min(255, int((d.b * s) + (original.b * c))),
      original.a
    );
  }
  fb.sprites = ships;
}

void update(uint32_t time) {
}

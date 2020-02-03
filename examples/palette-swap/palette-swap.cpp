#include "palette-swap.hpp"

#define NUM_PALETTES 4
#define SWATCH_SIZE 29

using namespace blit;

Size boar_ship_size(boar_ship[10], boar_ship[12]);

/*uint8_t __ss[64 * 64];
surface ss((uint8_t *)__ss, boar_ship_size, pixel_format::P);
spritesheet ss_boar_ship(ss, 8, 8);*/

RGBA alternate_palettes[NUM_PALETTES][5] = {
    {
        RGBA(87, 37, 59),
        RGBA(148, 54, 61),
        RGBA(213, 112, 51),
        RGBA(242, 182, 61),
        RGBA(247, 206, 123)
    },
    {
        RGBA(37, 38, 87),
        RGBA(85, 53, 148),
        RGBA(179, 51, 213),
        RGBA(242, 61, 226),
        RGBA(247, 123, 236)
    },
    {
        RGBA(87, 87, 87),
        RGBA(148, 148, 148),
        RGBA(195, 198, 212),
        RGBA(225, 223, 240),
        RGBA(245, 245, 245)
    },
    {
        RGBA(91, 21, 21),
        RGBA(134, 31, 31),
        RGBA(181, 21, 21),
        RGBA(233, 39, 39),
        RGBA(253, 76, 76)
    }
};

Point position[NUM_PALETTES] = {
    Point(30, 50),
    Point(180, 140),
    Point(50, 140),
    Point(180, 40)
};

void init() {
  set_screen_mode(ScreenMode::hires);

  //ss_boar_ship.s.load_from_packed(boar_ship);
  screen.sprites = SpriteSheet::load(boar_ship);
}

void all_ships_at_once_demo(uint32_t time) {
  screen.pen(RGBA(0x4e, 0xb3, 0xf7));
  screen.clear();

  screen.alpha = 255;
  screen.pen(RGBA(255, 255, 255));
  screen.rectangle(Rect(0, 0, 320, 14));
  screen.pen(RGBA(0, 0, 0));
  screen.text("Palette swap demo", &minimal_font[0][0], Point(5, 4));

  for (int p = 0; p < NUM_PALETTES; p++){
    for (int x = 0; x < 5; x++){
        screen.sprites->palette[4 + x] = alternate_palettes[p][x];
    }

    Point pos = position[p];

    pos.x += sin(time / 5000.0 + p) * 20;
    pos.y += sin(time / 500.0 + p) * 10;

    screen.stretch_blit(screen.sprites,
        Rect(0, 0, boar_ship_size.w, boar_ship_size.h),
        Rect(pos.x, pos.y, boar_ship_size.w * 2, boar_ship_size.h * 2)
    );
  }

  screen.watermark();
}

void single_ship_cycling_demo(uint32_t time) {
  screen.pen(RGBA(0x4e, 0xb3, 0xf7));
  screen.clear();

  screen.alpha = 255;
  screen.pen(RGBA(255, 255, 255));
  screen.rectangle(Rect(0, 0, 320, 14));
  screen.pen(RGBA(0, 0, 0));
  screen.text("Palette swap demo", &minimal_font[0][0], Point(5, 4));

  int palette_index = int(time / 1000.0) % NUM_PALETTES;

  for (int x = 0; x < 5; x++){
    screen.sprites->palette[4 + x] = alternate_palettes[palette_index][x];
  }

  screen.stretch_blit(screen.sprites,
    Rect(0, 0, boar_ship_size.w, boar_ship_size.h),
    Rect(
        (screen.bounds.w - boar_ship_size.w * 4) / 2,
        5 + (screen.bounds.h - ((SWATCH_SIZE + 10) * 2) - boar_ship_size.h * 4) / 2,
        boar_ship_size.w * 4, boar_ship_size.h * 4
    )
  );

  uint8_t top = screen.bounds.h - ((SWATCH_SIZE + 10) * 2);

  for (int p = 0; p < NUM_PALETTES; p++){

    uint8_t palette_top = p > 1 ? top + SWATCH_SIZE + 10 : top;
    uint8_t palette_left = ((p % 2) * 5 * SWATCH_SIZE) + (10 * (p % 2)) + 10;

    if (palette_index == p) {
        screen.pen(RGBA(0, 0, 0));
        screen.rectangle(Rect(
          palette_left - 4,
          palette_top - 4,
          SWATCH_SIZE * 5 + 8,
          SWATCH_SIZE + 8
        ));
    }
    for (int c = 0; c < 5; c++) {
      RGBA colour = alternate_palettes[p][c];
      screen.pen(colour);
      screen.rectangle(Rect(
          palette_left + (c * SWATCH_SIZE),
          palette_top,
          SWATCH_SIZE,
          SWATCH_SIZE
      )); 
    }
  }

  screen.watermark();
}

void render(uint32_t time) {
  all_ships_at_once_demo(time);
}

void update(uint32_t time) {

}
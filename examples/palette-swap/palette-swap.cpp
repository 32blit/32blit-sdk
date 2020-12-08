#include "palette-swap.hpp"
#include "assets.hpp"

#define NUM_PALETTES 4
#define SWATCH_SIZE 29

using namespace blit;

Pen alternate_palettes[NUM_PALETTES][5] = {
    {
        Pen(87, 37, 59),
        Pen(148, 54, 61),
        Pen(213, 112, 51),
        Pen(242, 182, 61),
        Pen(247, 206, 123)
    },
    {
        Pen(37, 38, 87),
        Pen(85, 53, 148),
        Pen(179, 51, 213),
        Pen(242, 61, 226),
        Pen(247, 123, 236)
    },
    {
        Pen(87, 87, 87),
        Pen(148, 148, 148),
        Pen(195, 198, 212),
        Pen(225, 223, 240),
        Pen(245, 245, 245)
    },
    {
        Pen(91, 21, 21),
        Pen(134, 31, 31),
        Pen(181, 21, 21),
        Pen(233, 39, 39),
        Pen(253, 76, 76)
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

  screen.sprites = SpriteSheet::load(boar_ship);
}

void all_ships_at_once_demo(uint32_t time) {
  screen.pen = Pen(0x4e, 0xb3, 0xf7);
  screen.clear();

  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));
  screen.pen = Pen(0, 0, 0);
  screen.text("Palette swap demo", minimal_font, Point(5, 4));

  for (int p = 0; p < NUM_PALETTES; p++){
    for (int x = 0; x < 5; x++){
        screen.sprites->palette[4 + x] = alternate_palettes[p][x];
    }

    Point pos = position[p];

    pos.x += sinf(time / 5000.0f + p) * 20.0f;
    pos.y += sinf(time / 500.0f + p) * 10.0f;

    screen.stretch_blit(screen.sprites,
        Rect(0, 0, screen.sprites->bounds.w, screen.sprites->bounds.h),
        Rect(pos.x, pos.y, screen.sprites->bounds.w * 2, screen.sprites->bounds.h * 2)
    );
  }

  screen.watermark();
}

void single_ship_cycling_demo(uint32_t time) {
  screen.pen = Pen(0x4e, 0xb3, 0xf7);
  screen.clear();

  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));
  screen.pen = Pen(0, 0, 0);
  screen.text("Palette swap demo", minimal_font, Point(5, 4));

  int palette_index = int(time / 1000.0) % NUM_PALETTES;

  for (int x = 0; x < 5; x++){
    screen.sprites->palette[4 + x] = alternate_palettes[palette_index][x];
  }

  screen.stretch_blit(screen.sprites,
    Rect(0, 0, screen.sprites->bounds.w, screen.sprites->bounds.h),
    Rect(
        (screen.bounds.w - screen.sprites->bounds.w * 4) / 2,
        5 + (screen.bounds.h - ((SWATCH_SIZE + 10) * 2) - screen.sprites->bounds.h * 4) / 2,
        screen.sprites->bounds.w * 4, screen.sprites->bounds.h * 4
    )
  );

  uint8_t top = screen.bounds.h - ((SWATCH_SIZE + 10) * 2);

  for (int p = 0; p < NUM_PALETTES; p++){

    uint8_t palette_top = p > 1 ? top + SWATCH_SIZE + 10 : top;
    uint8_t palette_left = ((p % 2) * 5 * SWATCH_SIZE) + (10 * (p % 2)) + 10;

    if (palette_index == p) {
        screen.pen = Pen(0, 0, 0);
        screen.rectangle(Rect(
          palette_left - 4,
          palette_top - 4,
          SWATCH_SIZE * 5 + 8,
          SWATCH_SIZE + 8
        ));
    }
    for (int c = 0; c < 5; c++) {
      Pen colour = alternate_palettes[p][c];
      screen.pen = colour;
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
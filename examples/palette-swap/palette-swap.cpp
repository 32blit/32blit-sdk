#include "palette-swap.hpp"

#define NUM_PALETTES 4
#define SWATCH_SIZE 29

using namespace blit;

size boar_ship_size(boar_ship[10], boar_ship[12]);

/*uint8_t __ss[64 * 64];
surface ss((uint8_t *)__ss, boar_ship_size, pixel_format::P);
spritesheet ss_boar_ship(ss, 8, 8);*/

rgba alternate_palettes[NUM_PALETTES][5] = {
    {
        rgba(87, 37, 59),
        rgba(148, 54, 61),
        rgba(213, 112, 51),
        rgba(242, 182, 61),
        rgba(247, 206, 123)
    },
    {
        rgba(37, 38, 87),
        rgba(85, 53, 148),
        rgba(179, 51, 213),
        rgba(242, 61, 226),
        rgba(247, 123, 236)
    },
    {
        rgba(87, 87, 87),
        rgba(148, 148, 148),
        rgba(195, 198, 212),
        rgba(225, 223, 240),
        rgba(245, 245, 245)
    },
    {
        rgba(91, 21, 21),
        rgba(134, 31, 31),
        rgba(181, 21, 21),
        rgba(233, 39, 39),
        rgba(253, 76, 76)
    }
};

point position[NUM_PALETTES] = {
    point(30, 50),
    point(180, 140),
    point(50, 140),
    point(180, 40)
};

void init() {
  set_screen_mode(screen_mode::hires);

  //ss_boar_ship.s.load_from_packed(boar_ship);
  fb.sprites = spritesheet::load(boar_ship);
}

void all_ships_at_once_demo(uint32_t time) {
  fb.pen(rgba(0x4e, 0xb3, 0xf7));
  fb.clear();

  fb.alpha = 255;
  fb.pen(rgba(255, 255, 255));
  fb.rectangle(rect(0, 0, 320, 14));
  fb.pen(rgba(0, 0, 0));
  fb.text("Palette swap demo", &minimal_font[0][0], point(5, 4));

  for (int p = 0; p < NUM_PALETTES; p++){
    for (int x = 0; x < 5; x++){
        fb.sprites->palette[4 + x] = alternate_palettes[p][x];
    }

    point pos = position[p];

    pos.x += sin(time / 5000.0 + p) * 20;
    pos.y += sin(time / 500.0 + p) * 10;

    fb.stretch_blit(fb.sprites,
        rect(0, 0, boar_ship_size.w, boar_ship_size.h),
        rect(pos.x, pos.y, boar_ship_size.w * 2, boar_ship_size.h * 2)
    );
  }

  fb.watermark();
}

void single_ship_cycling_demo(uint32_t time) {
  fb.pen(rgba(0x4e, 0xb3, 0xf7));
  fb.clear();

  fb.alpha = 255;
  fb.pen(rgba(255, 255, 255));
  fb.rectangle(rect(0, 0, 320, 14));
  fb.pen(rgba(0, 0, 0));
  fb.text("Palette swap demo", &minimal_font[0][0], point(5, 4));

  int palette_index = int(time / 1000.0) % NUM_PALETTES;

  for (int x = 0; x < 5; x++){
    fb.sprites->palette[4 + x] = alternate_palettes[palette_index][x];
  }

  fb.stretch_blit(fb.sprites,
    rect(0, 0, boar_ship_size.w, boar_ship_size.h),
    rect(
        (fb.bounds.w - boar_ship_size.w * 4) / 2,
        5 + (fb.bounds.h - ((SWATCH_SIZE + 10) * 2) - boar_ship_size.h * 4) / 2,
        boar_ship_size.w * 4, boar_ship_size.h * 4
    )
  );

  uint8_t top = fb.bounds.h - ((SWATCH_SIZE + 10) * 2);

  for (int p = 0; p < NUM_PALETTES; p++){

    uint8_t palette_top = p > 1 ? top + SWATCH_SIZE + 10 : top;
    uint8_t palette_left = ((p % 2) * 5 * SWATCH_SIZE) + (10 * (p % 2)) + 10;

    if (palette_index == p) {
        fb.pen(rgba(0, 0, 0));
        fb.rectangle(rect(
          palette_left - 4,
          palette_top - 4,
          SWATCH_SIZE * 5 + 8,
          SWATCH_SIZE + 8
        ));
    }
    for (int c = 0; c < 5; c++) {
      rgba colour = alternate_palettes[p][c];
      fb.pen(colour);
      fb.rectangle(rect(
          palette_left + (c * SWATCH_SIZE),
          palette_top,
          SWATCH_SIZE,
          SWATCH_SIZE
      )); 
    }
  }

  fb.watermark();
}

void render(uint32_t time) {
  all_ships_at_once_demo(time);
}

void update(uint32_t time) {

}
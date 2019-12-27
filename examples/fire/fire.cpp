
#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>


#include "fire.hpp"

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

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;

/* define storage for the framebuffer, spritesheet, and mask */
//rgb     __fb[screen_width * screen_height] __attribute__((section(".fb")));
#ifdef __APPLE__
uint8_t __m[screen_width * screen_height] __attribute__((section("__DATA,.m")));
uint8_t __pb[screen_width * screen_height] __attribute__((section("__DATA,.fb")));
#else
uint8_t __m[screen_width * screen_height] __attribute__((section(".m")));
uint8_t __pb[screen_width * screen_height] __attribute__((section(".fb")));
#endif
/* create surfaces */
//surface fb((uint8_t *)__fb, size(screen_width, screen_height), pixel_format::RGB);
surface m((uint8_t *)__m, pixel_format::M, size(screen_width, screen_height));
surface fbb((uint8_t *)__pb, pixel_format::P, size(screen_width, screen_height));

/* setup */
void init() {
  /*
      fb.alpha = 255;
      fb.mask = nullptr;
      fb.pen(rgba(39, 39, 54));
      fb.clear();*/
}


void hblur(surface m) {
  uint8_t last;

  uint8_t *p = (uint8_t *)m.data;
  for (uint8_t y = 0; y < 120; y++) {
    last = *p;
    p++;

    for (uint8_t x = 1; x < 159; x++) {
      *p = (*(p + 1) + last + *p + *p) >> 2;
      last = *p;
      p++;
    }

    p++;
  }

  // vertical

  for (uint8_t x = 0; x < 160; x++) {
    uint8_t *p = (uint8_t *)m.data + x;

    last = *p;
    p += 160;

    for (uint8_t y = 1; y < 119; y++) {
      *p = (*(p + 160) + last + *p + *p) >> 2;
      last = *p;
      p += 160;
    }
  }
}

void fire(uint32_t time_ms) {
  // palette based surface for rendering fire effect into
  static uint8_t tmp[320 * 240];

  if (fbb.palette.size() == 0) {
    fbb.palette.resize(256);

    for (int i = 0; i < 256; i++) {
      fbb.palette[i] = rgba(0, 0, 0);

      if (i > 0)
        fbb.palette[i] = rgba(62, 39, 49);

      if (i > 32)
        fbb.palette[i] = rgba(162, 38, 51);

      if (i > 64)
        fbb.palette[i] = rgba(228, 59, 68);

      if (i > 96)
        fbb.palette[i] = rgba(247, 118, 34);

      if (i > 128)
        fbb.palette[i] = rgba(254, 231, 97);
    }
  }

  rgb black = rgb(0, 0, 0);

  // seed the fire

  for (int i = 0; i < 400; i++) {
    point p(rand() % 320, (rand() % 10) + 220);
    fbb.pen(rgba(rand()));
    fbb.pixel(p);
  }

  // add sparks
  for (int i = 0; i < 5; i++) {
    point p(rand() % 320, rand() % 240);
    fbb.pen(rgba(rand()));
    fbb.pixel(p);
  }


  float ft = float(time_ms) / 1000.0f;

  uint8_t *pdest = fbb.data + fb.bounds.w;
  point p;
  for (p.y = 1; p.y < fb.bounds.h; p.y++) {
    int xoff = sin(float(p.y) / 10.0f + ft) * 2;

    for (p.x = 0; p.x < fb.bounds.w; p.x++) {
      uint8_t c = *pdest;

      if (c == 0) {
        *(pdest - fb.bounds.w) = 0;
      }
      else {
        uint8_t rnd_idx = rand() & 0b11;

        uint8_t *pf = pdest - rnd_idx + 2 + xoff - fb.bounds.w;

        int r = c - (rnd_idx % 2);
        r = r < 0 ? 0 : r;
        *pf = r;
      }

      pdest++;
    }
  }



  memcpy(tmp, fbb.data, 160 * 120);
  //  hblur(fbb);
  //  hblur(fbb);
  //  hblur(fbb);
    // hblur(fbb);
  hblur(fbb);

  //  hblur(fbb);

  m.pen(rgba(0));
  m.clear();
  m.pen(rgba(255));

  int logo_size = 16;
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      if (logo[y][x] != 0) {
        m.rectangle(rect(160 - (8 * logo_size) + (x * logo_size) + 8, 120 - (8 * logo_size) + (y * logo_size) + 8, logo_size, logo_size));
      }
    }
  }




  // screen.mask = &m;

  fb.blit(&fbb, fb.clip, fb.clip.tl());

  fb.pen(rgba(0, 255, 0));

  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      if (logo[y][x] == 2) {
        fb.rectangle(rect(80 - (8 * logo_size) + (x * logo_size) + 4, 60 - (8 * logo_size) + (y * logo_size) + 4, logo_size, logo_size));
      }
    }
  }

  memcpy(fbb.data, tmp, 160 * 120);
  fb.mask = nullptr;

}

void rotozoom(uint32_t time_ms) {
  static float angle = 0.0f;

  static rgba palette[] = { rgba(0, 0, 0), rgba(255, 255, 255), rgba(0, 255, 0) };
  //static rgb565 palette[] = { rgb565(0, 0, 0), rgb565(255, 255, 255), rgb565(0, 255, 0) };

  float c = cos(angle * M_PI / 180.0f);
  float s = sin(angle * M_PI / 180.0f);

  angle += 0.25f;
  angle = angle >= 360.0f ? 0.0f : angle;

  point p;

  int16_t w = fb.bounds.w;
  int16_t h = fb.bounds.h;

  int16_t hw = w / 2;
  int16_t hh = h / 2;

  //rgb5 *pdest = (rgb565 *)fb.data;
  uint32_t o = 0;
  for (p.y = 0; p.y < h; p.y++) {
    for (p.x = 0; p.x < w; p.x++) {
      uint8_t u = int16_t(float((p.x - hw) * c - (p.y - hh) * s) * s) & 0b1111;
      uint8_t v = int16_t(float((p.x - hw) * s + (p.y - hh) * c) * s) & 0b1111;

      uint8_t pi = logo[15 - u][v];

      //fb.pen(palette[pi]);
      //fb.pixel(p);
      fb.bf((uint8_t *)&palette[pi], &fb, o, 1);

      o++;
      //*pdest++ = palette[pi];
    }
  }
}

void moire(uint32_t time_ms) {
  /* float time = float(time_ms) / 200.0f;

    int16_t cx1 = sin(time / 2.0f) * fb.bounds.w / 3.0f + fb.bounds.w / 2.0f;
    int16_t cy1 = sin(time / 4.0f) * fb.bounds.h / 3.0f + fb.bounds.h / 2.0f;
    int16_t cx2 = cos(time / 3.0f) * fb.bounds.w / 3.0f + fb.bounds.w / 2.0f;
    int16_t cy2 = cos(time / 1.0f) * fb.bounds.h / 3.0f + fb.bounds.h / 2.0f;

    fb.pen(rgba(255, 255, 255));

    point p;
    rgb black = rgb(0, 0, 0);
    rgb white = rgb(255, 255, 255);
    rgb *pdest = (rgb *)fb.data;
    for (p.y = 0; p.y < fb.bounds.h; p.y++) {
      int dy = (p.y - cy1) * (p.y - cy1);
      int dy2 = (p.y - cy2) * (p.y - cy2);

      for (p.x = 0; p.x < fb.bounds.w; p.x++) {
        int dx = (p.x - cx1) * (p.x - cx1);
        int dx2 = (p.x - cx2) * (p.x - cx2);

        int sd = int(sqrtf(dx + dy));
        int sd2 = int(sqrtf(dx2 + dy2));
        int bwxor = (sd ^ sd2) >> 4;
        int shade = (bwxor & 1) * 255;

        *pdest++ = shade ? white : black;
      }
    }*/
}

int tick_count = 0;
void render(uint32_t time_ms) {


  tick_count++;


  //   fb.pen(rgba(0, 0, 0));
  //   fb.clear();

      //moire(time_ms);
      //fire(time_ms);
  fb.alpha = 255;
  fb.mask = nullptr;

  uint32_t ms_start = now();

  rotozoom(time_ms);

  uint32_t ms_end = now();
  
  /*
  fb.pen(rgba(200, 150, 100, 200));
  fb.rectangle(rect(5, 5, 310, 230));

  fb.pen(rgba(0, 0, 0, 100));
  fb.text("Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ac molestie eros. Nulla malesuada, nisl sit amet dictum ultrices, quam odio dictum tellus, euismod pulvinar felis sem ut ipsum. Quisque mauris mi, egestas nec ex at, fermentum sodales nulla. Quisque fermentum lectus sed vulputate rhoncus. Nulla pharetra rutrum neque, efficitur efficitur massa pharetra et. Suspendisse tincidunt libero eu ornare consequat.", &minimal_font[0][0], rect(11, 11, 300, 100));
  fb.text("Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ac molestie eros. Nulla malesuada, nisl sit amet dictum ultrices, quam odio dictum tellus, euismod pulvinar felis sem ut ipsum. Quisque mauris mi, egestas nec ex at, fermentum sodales nulla. Quisque fermentum lectus sed vulputate rhoncus. Nulla pharetra rutrum neque, efficitur efficitur massa pharetra et. Suspendisse tincidunt libero eu ornare consequat.", &outline_font[0][0], rect(11, 81, 300, 100));
  fb.text("Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ac molestie eros. Nulla malesuada, nisl sit amet dictum ultrices, quam odio dictum tellus, euismod pulvinar felis sem ut ipsum. Quisque mauris mi, egestas nec ex at, fermentum sodales nulla. Quisque fermentum lectus sed vulputate rhoncus. Nulla pharetra rutrum neque, efficitur efficitur massa pharetra et. Suspendisse tincidunt libero eu ornare consequat.", &fat_font[0][0], rect(11, 161, 300, 100));
  fb.pen(rgba(255, 255, 255));
  fb.text("Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ac molestie eros. Nulla malesuada, nisl sit amet dictum ultrices, quam odio dictum tellus, euismod pulvinar felis sem ut ipsum. Quisque mauris mi, egestas nec ex at, fermentum sodales nulla. Quisque fermentum lectus sed vulputate rhoncus. Nulla pharetra rutrum neque, efficitur efficitur massa pharetra et. Suspendisse tincidunt libero eu ornare consequat.", &minimal_font[0][0], rect(10, 10, 300, 100));
  fb.text("Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ac molestie eros. Nulla malesuada, nisl sit amet dictum ultrices, quam odio dictum tellus, euismod pulvinar felis sem ut ipsum. Quisque mauris mi, egestas nec ex at, fermentum sodales nulla. Quisque fermentum lectus sed vulputate rhoncus. Nulla pharetra rutrum neque, efficitur efficitur massa pharetra et. Suspendisse tincidunt libero eu ornare consequat.", &outline_font[0][0], rect(10, 80, 300, 100));
  fb.text("Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ac molestie eros. Nulla malesuada, nisl sit amet dictum ultrices, quam odio dictum tellus, euismod pulvinar felis sem ut ipsum. Quisque mauris mi, egestas nec ex at, fermentum sodales nulla. Quisque fermentum lectus sed vulputate rhoncus. Nulla pharetra rutrum neque, efficitur efficitur massa pharetra et. Suspendisse tincidunt libero eu ornare consequat.", &fat_font[0][0], rect(10, 160, 300, 100));
  */

  // draw FPS meter
  fb.pen(rgba(0, 0, 0, 200));
  fb.rectangle(rect(5, 5, 20, 16));
  fb.pen(rgba(255, 0, 0));
  std::string fms = std::to_string(ms_end - ms_start);
  fb.text(fms, &minimal_font[0][0], rect(10, 10, 10, 16));

  int block_size = 4;
  for (int i = 0; i < (ms_end - ms_start); i++) {
    fb.pen(rgba(i * 5, 255 - (i * 5), 0));
    fb.rectangle(rect(i * (block_size + 1) + 1, fb.bounds.h - block_size - 1, block_size, block_size));
  }

}

void update(uint32_t time) {


  if (pressed(button::A)) {
    set_screen_mode(screen_mode::lores);
  }
  else {
    set_screen_mode(screen_mode::hires);
  }

}

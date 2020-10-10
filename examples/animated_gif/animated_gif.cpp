
#include <cstring>

#include "animated_gif.hpp"
#define __LINUX__
#include "AnimatedGIF.h"
#include "badger.h"

using namespace blit;

AnimatedGIF gif; // static class instance
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
static uint8_t image[DISPLAY_WIDTH * DISPLAY_HEIGHT]; // holds the 8-bit GIF image
static uint8_t palTemp[256*3];

// Draw a line of image into memory and send the whole line to the display
void GIFDraw(GIFDRAW *pDraw)
{
    uint8_t *s, *d;
    uint16_t *pus, us;
    int x, y;

    if (pDraw->y == 0) // first line, get palette as 24-bpp
    {
       pus = pDraw->pPalette;
       d = palTemp;
       for (x=0; x<256; x++)
       {
          us = *pus++; // get RGB565 palette entry
          *d++ = ((us >> 8) & 0xf8) | (us >> 13); // R
          *d++ = ((us >> 3) & 0xfc) | ((us >> 9) & 0x3); // G
          *d++ = ((us & 0x1f) << 3) | ((us >> 2) & 0x7); // B 
       }
    }
    y = pDraw->iY + pDraw->y; // current line
    if (y >= DISPLAY_HEIGHT)
       return;
    s = pDraw->pPixels;
    d = &image[(DISPLAY_WIDTH * y) + pDraw->iX];
    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) // if transparency used
    {
      uint8_t c, ucTransparent = pDraw->ucTransparent;
      for (x=0; x<pDraw->iWidth; x++)
      {
          c = *s++;
          if (c != ucTransparent)
             d[x] = c;
      }
    }
    else
    {
      memcpy(d, s, pDraw->iWidth); // copy all of the pixels
    }
    s = &image[(DISPLAY_WIDTH * y) + pDraw->iX];
    // Translate the 8-bit pixels through the palette
    d = (uint8_t *)screen.data;
    d += (y * DISPLAY_WIDTH * 3) + (pDraw->iX * 3);
    for (x=0; x<pDraw->iWidth; x++)
    {
      uint8_t *ppal = &palTemp[s[0] * 3];
      s++;
      d[0] = *ppal++;
      d[1] = *ppal++;
      d[2] = *ppal++;
      d += 3;
    }
} /* GIFDraw() */

/* setup */
void init() {
  set_screen_mode(ScreenMode::hires);

  gif.begin(LITTLE_ENDIAN_PIXELS);
  gif.open((uint8_t *)badger, sizeof(badger), GIFDraw);

}

void render(uint32_t time) {
}

void update(uint32_t time) {
static int iTicks = 0;

   if (iTicks > 0)
   {
      iTicks -= 10;
      return;
   }
   if (!gif.playFrame(false, &iTicks))
     gif.reset();
}

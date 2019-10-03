
#include <string>

#include "../types/pixel_format.hpp"
#include "../types/point.hpp"
#include "../types/rect.hpp"
#include "font.hpp"

using namespace blit;

namespace blit {  

  //  draws text in the current pen colour using the default font

  int32_t text(std::string message, const rect &r) {
    point c(r.x, r.y); // caret position
    /*
    rgba cols[4] = {
      rgba(pen.r, pen.g, pen.b, (0 * pen.a) / 255),
      rgba(pen.r, pen.g, pen.b, (255 * pen.a) / 255),
      rgba(pen.r, pen.g, pen.b, (169 * pen.a) / 255),
      rgba(pen.r, pen.g, pen.b, (84 * pen.a) / 255)
    };

    for (char &chr : message) {
      // draw character
      const uint16_t *chr_span = &(font_8x8_2bpp[chr * 8]);

      uint8_t char_width = 0;


      for (uint8_t i = 0; i < 8; i++) { 
        uint8_t *pd = target->ptr(c.x, c.y + i);         

        for (uint8_t j = 0; j < 8; j++) {
          if(clip.contains(point(c.x + j, c.y + i))) {          
            // get font value
            uint16_t mask = 0b11 << (16 - (j << 1));
            uint8_t color_index = (*chr_span & mask) >> (16 - (j << 1));

            if (color_index != 0) {
              color((uint8_t *)&cols[color_index], pd);
              char_width = char_width < j ? j : char_width;
            }
          }

          pd += target->stride;
        }

        chr_span++;
        pd += target->row_stride;
      }

      if (chr == 32) {
        char_width = 3;
      }

      // increment the cursor
      c.x += char_width;
      if ((c.x > r.x + r.w) | (chr == 10)) {
        c.x = r.x;
        c.y += 9;
      }
    }*/

    return c.y + 8;
  }  
}

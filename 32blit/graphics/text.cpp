
#include <string>

#include "../types/pixel_format.hpp"
#include "../types/point.hpp"
#include "../types/rect.hpp"
#include "font.hpp"
#include "surface.hpp"

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

  /**
   * TODO: Document this function
   *
   * \param message
   * \param font
   * \param p
   * \param variable
   */
  void surface::text(std::string message, const uint8_t *font, const point &p, bool variable, text_align align) {
    text(message, font, rect(p.x, p.y, 10000, 10000), variable, align);
  }

  /**
   * TODO: Document this function
   *
   * \param message
   * \param font
   * \param r
   * \param variable
   */
  void surface::text(std::string message, const uint8_t *font, const rect &r, bool variable, text_align align) {
    point c(r.x, r.y); // caret position

    // check vertical alignment
    if ((align & 0b11) != blit::text_align::top) {
      size bounds = measure_text(message, font, variable);

      if ((align & 0b11) == text_align::bottom)
        c.y += r.h - bounds.h;
      else // center
        c.y += (r.h - bounds.h) / 2;
    }

    // check horizontal alignment
    if ((align & 0b1100) != blit::text_align::left) {
      size bounds = measure_text(message.substr(0, message.find_first_of('\n')), font, variable);

      if ((align & 0b1100) == text_align::right)
        c.x += r.w - bounds.w;
      else // center
        c.x += (r.w - bounds.w) / 2;
    }

    size_t char_off = 0;
    for (char &chr : message) {
      // draw character

      uint8_t chr_idx = chr & 0x7F;
      chr_idx = chr_idx < ' ' ? 0 : chr_idx - ' ';

      uint8_t char_width = 0;

      const uint8_t* font_chr = &font[chr_idx * 6];

      for (uint8_t y = 0; y < 8; y++) {
        uint32_t po = offset(point(c.x, c.y + y));

        for (uint8_t x = 0; x < 6; x++) {

          //if(clip.contains(p)) {
            if (font_chr[x] & (1 << y)) {
              bf((uint8_t *)&_pen, this, po, 1);
              char_width = char_width < x ? x : char_width;
            }
          //}

          //p.x++;
          po++;
        }

        //p.y++;
      }

      if (!variable)
        char_width = 4;

      char_width += 2;

      if (chr == ' ' && variable) {
        char_width = 3;
      }

      // increment the cursor
      c.x += char_width;
      if ((c.x > r.x + r.w) | (chr == 10)) {
        c.x = r.x;
        c.y += 9;

        // check horizontal alignment
        if ((align & 0b1100) != blit::text_align::left) {
          auto end = message.find_first_of('\n', char_off + 1);
          if(end != std::string::npos)
            end -= char_off + 1;

          size bounds = measure_text(message.substr(char_off + 1, end), font, variable);

          if ((align & 0b1100) == text_align::right)
            c.x += r.w - bounds.w;
          else // center
            c.x += (r.w - bounds.w) / 2;
        }
      }

      char_off++;
    }

    //return c.y + 8;
  }

  uint8_t get_char_width(const uint8_t *font, char c, bool variable) {
    const int fixed_char_width = 6;
    if (!variable)
      return fixed_char_width;

    if (c == ' ')
      return 3;

    uint8_t char_width = 0;
    uint8_t chr_idx = c & 0x7F;
    chr_idx = chr_idx < ' ' ? 0 : chr_idx - ' ';

    const uint8_t* font_chr = &font[chr_idx * 6];

    for (uint8_t y = 0; y < 8; y++) {
      for (uint8_t x = 0; x < 6; x++) {
        if (font_chr[x] & (1 << y)) {
          char_width = char_width < x ? x : char_width;
        }
      }
    }

    return char_width + 2;
  }

  size surface::measure_text(std::string message, const uint8_t *font, bool variable) {
    const int fixed_char_width = 6;
    const int line_height = 9;

    size bounds(0, 0);

    size_t char_off = 0;
    int line_len = 0;

    while (char_off < message.length()) {
      if (variable) {
        line_len += get_char_width(font, message[char_off], true);
        char_off++;
      } else {
        // calculate a line at a time if using fixed-width characters
        size_t end = message.find_first_of('\n', char_off);
        if (end == std::string::npos)
          end = message.length();

        line_len = (end - char_off) * fixed_char_width;
        char_off = end;
      }

      // new line/end
      if (char_off == message.length() || message[char_off] == '\n') {
        if (line_len > bounds.w)
          bounds.w = line_len;

        bounds.h += line_height;

        line_len = 0;
        char_off++;
      }
    }

    return bounds;
  }
}

std::string surface::wrap_text(std::string message, int32_t width, const uint8_t *font, bool variable) {
  std::string ret;

  int current_x = 0;
  size_t last_space = std::string::npos;
  size_t copied_off = 0;

  for (size_t i = 0; i < message.length(); i++) {
    if (message[i] == ' ')
      last_space = i;

    current_x += get_char_width(font, message[i], variable);

    if (current_x > width) {
      if(last_space == std::string::npos) {
        // no space to break at
        ret += message.substr(copied_off, i - copied_off - 1) + "\n";
        copied_off = i - 1;
      } else {
        // break at last space
        ret += message.substr(copied_off, last_space - copied_off) + "\n";
        copied_off = last_space + 1; // don't copy the space
        last_space = std::string::npos;
      }
      current_x = 0;
    }
  }

  ret += message.substr(copied_off);

  return ret;
}

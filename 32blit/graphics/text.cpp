
#include <string>

#include "../types/point.hpp"
#include "../types/rect.hpp"
#include "font.hpp"
#include "surface.hpp"

using namespace blit;

namespace blit {

  /**
   * TODO: Document this function
   *
   * \param message
   * \param font
   * \param p
   * \param variable
   */
  void Surface::text(const std::string &message, const Font &font, const Point &p, bool variable, TextAlign align, Rect clip) {
    text(message, font, Rect(p.x, p.y, 0, 0), variable, align, clip);
  }

  /**
   * TODO: Document this function
   *
   * \param message
   * \param font
   * \param r
   * \param variable
   */
  void Surface::text(const std::string &message, const Font &font, const Rect &r, bool variable, TextAlign align, Rect clip) {
    Point c(r.x, r.y); // caret position

    // default clip rect to rect if passed in
    if(r.w > 0 && clip.w == 1000)
      clip = r;

    // clamp clip rect to screen
    clip.w = std::min(clip.w, bounds.w - clip.x);
    clip.h = std::min(clip.h, bounds.h - clip.y);

    // check vertical alignment
    if ((align & 0b11) != TextAlign::top) {
      Size bounds = measure_text(message, font, variable);

      if ((align & 0b11) == TextAlign::bottom)
        c.y += r.h - bounds.h;
      else // center
        c.y += (r.h - bounds.h) / 2;
    }

    // check horizontal alignment
    if ((align & 0b1100) != TextAlign::left) {
      Size bounds = measure_text(message.substr(0, message.find_first_of('\n')), font, variable);

      if ((align & 0b1100) == TextAlign::right)
        c.x += r.w - bounds.w;
      else // center
        c.x += (r.w - bounds.w) / 2;
    }

    const int height_bytes = (font.char_h + 7) / 8;
    const int char_size = font.char_w * height_bytes;

    size_t char_off = 0;
    for (char chr : message) {
      // draw character

      uint8_t chr_idx = chr & 0x7F;
      chr_idx = chr_idx < ' ' ? 0 : chr_idx - ' ';

      uint8_t char_width = 0;

      const uint8_t* font_chr = &font.data[chr_idx * char_size];

      for (uint8_t y = 0; y < font.char_h; y++) {
        uint32_t po = offset(Point(c.x, c.y + y));

        for (uint8_t x = 0; x < font.char_w; x++) {
          int bit = 1 << (y & 7);
          if (font_chr[x * height_bytes + y / 8] & bit) {
            if(clip.contains(Point(c.x + x, c.y + y)))
              pbf(&pen, this, po, 1);
          }

          po++;
        }
      }

      if (!variable)
        char_width = font.char_w;
      else
        char_width = font.char_w_variable[chr_idx];

      // increment the cursor
      c.x += char_width;
      if (chr == 10) {
        c.x = r.x;
        c.y += font.char_h + font.spacing_y;

        // check horizontal alignment
        if ((align & 0b1100) != TextAlign::left) {
          auto end = message.find_first_of('\n', char_off + 1);
          if(end != std::string::npos)
            end -= char_off + 1;

          Size bounds = measure_text(message.substr(char_off + 1, end), font, variable);

          if ((align & 0b1100) == TextAlign::right)
            c.x += r.w - bounds.w;
          else // center
            c.x += (r.w - bounds.w) / 2;
        }
      }

      char_off++;
    }
  }

  uint8_t get_char_width(const Font &font, char c, bool variable) {
    if (!variable)
      return font.char_w;

    uint8_t chr_idx = c & 0x7F;
    chr_idx = chr_idx < ' ' ? 0 : chr_idx - ' ';

    return font.char_w_variable[chr_idx];
  }

  Size Surface::measure_text(const std::string &message, const Font &font, bool variable) {
    const int line_height = font.char_h + font.spacing_y;

    Size bounds(0, 0);

    size_t char_off = 0;
    int line_len = 0;

    while (char_off < message.length()) {
      // newline
      if (message[char_off] == '\n') {
        if (line_len > bounds.w)
          bounds.w = line_len;

        bounds.h += line_height;

        line_len = 0;
        char_off++;
      } else if (variable) {
        line_len += get_char_width(font, message[char_off], true);
        char_off++;
      } else {
        // calculate a line at a time if using fixed-width characters
        size_t end = message.find_first_of('\n', char_off);
        if (end == std::string::npos)
          end = message.length();

        line_len = (end - char_off) * font.char_w;
        char_off = end;
      }
    }

    // update for final line
    if (line_len > bounds.w)
      bounds.w = line_len;

    bounds.h += line_height;

    return bounds;
  }
}

std::string Surface::wrap_text(const std::string &message, int32_t width, const Font &font, bool variable, bool words) {
  std::string ret;

  int current_x = 0;
  size_t last_space = std::string::npos;
  size_t copied_off = 0;

  for (size_t i = 0; i < message.length(); i++) {
    if (message[i] == ' ')
      last_space = i;

    if(message[i] == '\n') {
      ret += message.substr(copied_off, i - copied_off);
      copied_off = i;
      current_x = 0;
      last_space = std::string::npos;
      continue;
    }

    int char_width = get_char_width(font, message[i], variable);
    current_x += char_width;

    if (current_x > width) {
      if(!words || last_space == std::string::npos) {
        // no space to break at or we're not breaking on words
        ret += message.substr(copied_off, i - copied_off - 1) + "\n";
        copied_off = i - 1;
        current_x = char_width;
      } else {
        // break at last space
        ret += message.substr(copied_off, last_space - copied_off) + "\n";
        copied_off = last_space + 1; // don't copy the space
        last_space = std::string::npos;
        current_x = measure_text(message.substr(copied_off, i - copied_off + 1), font, variable).w;
      }
    }
  }

  ret += message.substr(copied_off);

  return ret;
}

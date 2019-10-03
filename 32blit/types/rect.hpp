#pragma once

#include <algorithm>
#include <stdint.h>

#include "point.hpp"
#include "size.hpp"

namespace blit {

  struct rect {

    int32_t x, y, w, h;

    rect() : x(0), y(0), w(0), h(0) {}
    rect(point tl, point br) : x(tl.x), y(tl.y), w(br.x - tl.x), h(br.y - tl.y) {}
    rect(point tl, size s) : x(tl.x), y(tl.y), w(s.w), h(s.h) {}
    rect(int32_t x, int32_t y, int32_t w, int32_t h) : x(x), y(y), w(w), h(h) {}

    inline rect& operator*= (const float a) { x *= a; y *= a; w *= a; h *= a; return *this; }

    bool empty() const {
      return w <= 0 || h <= 0; 
    }

    bool contains(const point &p) const {
      return p.x >= x && p.y >= y && p.x < x + w && p.y < y + h; 
    }

    bool contains(const rect &p) const {
      return p.x >= x && p.y >= y && p.x + p.w < x + w && p.y + p.h < y + h;
    }

    bool intersects(const rect &r) const {
      return !(x > r.x + r.w || x + w < r.x || y > r.y + r.h || y + h < r.y); 
    }

    rect intersection(const rect &r) const {
      return rect(
        std::max(x, r.x), 
        std::max(y, r.y), 
        std::min(x + w, r.x + r.w) - std::max(x, r.x), 
        std::min(y + h, r.y + r.h) - std::max(y, r.y));
    }

    // TODO: hate this function name
    //
    // clips a destination rect to fit into the clipping rectangle
    // also offsets and scales the src rect equivalently.
    // modifies the supplied src/dest rects
    //
    // user should check if dest is empty (in which case src is
    // not modified) to determine if intersection occurred
    //
    // usage: clip.clip_src_dest(src, dest)
    //
    // !! ouch, this description is poor.
  /*  void clip_src_dest(rect &src, rect &dest) {
      rect cdr = intersection(dest);  // clipped destination rect

      if (cdr.empty())
        return; // after clipping there is nothing to draw      

      float sx = (src.w) / float(dest.w);
      float sy = (src.h) / float(dest.h);

      // offset source rect to accomodate for clipped destination rect    
      uint8_t l = cdr.x - dest.x; // top left corner
      uint8_t t = cdr.y - dest.y;

      src.x += (sx * l);
      src.y += (sy * t);

      uint8_t w = cdr.w - dest.w;
      uint8_t h = cdr.h - dest.h;

      src.w = cdr.w * sx;
      src.h = cdr.h * sy;
    }*/

    void deflate(int32_t v) { 
      x += v; 
      y += v; 
      w -= 2 * v;
      h -= 2 * v; 
    }

    void inflate(int32_t v) {
      x -= v;
      y -= v;
      w += 2 * v;
      h += 2 * v;
    }

    point clamp(point p) const {
      return point(
        p.x < x ? x : (p.x > x + w ? x + w : p.x), 
        p.y < y ? y : (p.y > y + h ? y + h : p.y)); 
    }

    point tl() const {
      return point(x, y);
    }

    point tr() const {
      return point(x + w, y);
    }

    point bl() const {
      return point(x, y + h);
    }

    point br() const {
      return point(x + w, y + h);
    }

  };

  inline rect operator*  (rect lhs, const float a) { lhs *= a; return lhs; }

}
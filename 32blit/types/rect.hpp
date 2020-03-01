#pragma once

#include <algorithm>
#include <cstdint>

#include "point.hpp"
#include "size.hpp"

namespace blit {

  struct Rect {

    int32_t x = 0, y = 0, w = 0, h = 0;

    Rect() = default;
    Rect(Point tl, Point br) : x(tl.x), y(tl.y), w(br.x - tl.x), h(br.y - tl.y) {}
    Rect(Point tl, Size s) : x(tl.x), y(tl.y), w(s.w), h(s.h) {}
    Rect(int32_t x, int32_t y, int32_t w, int32_t h) : x(x), y(y), w(w), h(h) {}

    inline Rect& operator*= (const float a) { x *= a; y *= a; w *= a; h *= a; return *this; }

    bool empty() const {
      return w <= 0 || h <= 0; 
    }

    bool contains(const Point &p) const {
      return p.x >= x && p.y >= y && p.x < x + w && p.y < y + h; 
    }

    bool contains(const Rect &p) const {
      return p.x >= x && p.y >= y && p.x + p.w < x + w && p.y + p.h < y + h;
    }

    bool intersects(const Rect &r) const {
      return !(x > r.x + r.w || x + w < r.x || y > r.y + r.h || y + h < r.y); 
    }

    Rect intersection(const Rect &r) const {
      return Rect(
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

    Point clamp(Point p) const {
      return Point(
        p.x < x ? x : (p.x > x + w ? x + w : p.x), 
        p.y < y ? y : (p.y > y + h ? y + h : p.y)); 
    }

    Point tl() const {
      return Point(x, y);
    }

    Point tr() const {
      return Point(x + w, y);
    }

    Point bl() const {
      return Point(x, y + h);
    }

    Point br() const {
      return Point(x + w, y + h);
    }

  };

  inline Rect operator*  (Rect lhs, const float a) { lhs *= a; return lhs; }

}
/*! \file primitive.cpp
    \brief Drawing routines for primitive shapes.
*/

#include <cstdlib>
#include <math.h>

#include "surface.hpp"

namespace blit {

  /**
   * Clear the surface to the current pen colour.
   */
  void Surface::clear() {
    rectangle(clip);
  }

  /**
   * Draw a filled rectangle in the current pen colour.
   *
   * \param[in] r `Rect` describing the desired rectangle.
   */
  void Surface::rectangle(const Rect &r) {
    Rect cr = clip.intersection(r);
    if (cr.empty())
      return;

    uint32_t o = offset(cr);

    for (uint8_t y = cr.y; y < cr.y + cr.h; y++) {
      pbf(&pen, this, o, cr.w);
      o += bounds.w;
    }
  }

  /**
   * Put a single pixel in the current pen colour.
   *
   * \param[in] p `Point` describing the pixel location.
   */
  void Surface::pixel(const Point &p) {
    if (!clip.contains(p))
      return;

    pbf(&pen, this, offset(p), 1);
  }

  /**
   * Draw a fast vertical line in the current pen colour.
   *
   * \param[in] p `Point` describing start of the line.
   * \ param[in] c `Count` of pixels to draw.
   */
  void Surface::v_span(Point p, int16_t c) {
    if (p.x < 0 || p.x >= bounds.w)
      return;

    if (p.y < 0) {
      c -= -p.y;
      p.y = 0;
    }

    if (p.y + c > bounds.h) {
      c -= (p.y + c - bounds.h);
    }

    while (c > 0) {
      pbf(&pen, this, offset(p), 1);
      p.y++;
      c--;
    }
  }

  /**
   * Draw a fast horizontal line in the current pen colour.
   *
   * \param[in] p `Point` describing start of the line.
   * \ param[in] c `Count` of pixels to draw.
   */
  void Surface::h_span(Point p, int16_t c) {
    if (p.y < 0 || p.y >= bounds.h)
      return;

    if (p.x < 0) {
      c -= -p.x;
      p.x = 0;
    }

    if (p.x + c > bounds.w) {
      c -= (p.x + c - bounds.w);
    }

    if (c > 0) {
      pbf(&pen, this, offset(p), c);
    }
  }

  /**
   * Draw a circle in the current pen colour.
   *
   * \param[in] c `Point` describing the center of the circle.
   * \param[in] r Radius of the circle.
   */
  void Surface::circle(const Point &c, int32_t r) {
    // if circle completely out of bounds then don't bother!
    if (!clip.intersects(Rect(c.x - r, c.y - r, r * 2, r * 2))) {
      return;
    }

    int x = r, y = 0, err = -r;
    while (x >= y)
    {
      int lastY = y;

      err += y; y++; err += y;

      h_span(Point(c.x - x, c.y + lastY), x * 2);
      if (lastY != 0) {
        h_span(Point(c.x - x, c.y - lastY), x * 2);
      }

      if (err >= 0) {
        if (x != lastY) {
          h_span(Point(c.x - lastY, c.y + x), lastY * 2);
          if (x != 0) {
            h_span(Point(c.x - lastY, c.y - x), lastY * 2);
          }

          err -= x; x--; err -= x;
        }
      }
    }
  }

  /**
   * Draw a line in the current pen colour.
   *
   * \param[in] p1 `Point` describing the start of the line.
   * \param[in] p2 `Point` describing the end of the line.
   */
  void Surface::line(const Point &p1, const Point &p2) {
    int32_t dx = int32_t(abs(p2.x - p1.x));
    int32_t dy = -int32_t(abs(p2.y - p1.y));

    int32_t sx = (p1.x < p2.x) ? 1 : -1;
    int32_t sy = (p1.y < p2.y) ? 1 : -1;

    int32_t err = dx + dy;

    Point p(p1);

    while (true) {
      if (clip.contains(p)) {
        pbf(&pen, this, offset(p), 1);
      }

      if ((p.x == p2.x) && (p.y == p2.y)) break;

      int32_t e2 = err * 2;
      if (e2 >= dy) { err += dy; p.x += sx; }
      if (e2 <= dx) { err += dx; p.y += sy; }
    }
  }

  /**
   * TODO: Document this function
   *
   * \param[in] p1
   * \param[in] p2
   * \param[in] p3
   */
  int32_t orient2d(Point p1, Point p2, Point p3) {
    return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
  }
  
  /**
   * TODO: Document this function
   *
   * \param[in] p1
   * \param[in] p2
   */
  bool is_top_left(const Point &p1, const Point &p2) {
    return (p1.y == p2.y && p1.x > p2.x) || (p1.y < p2.y);
  }

  /**
   * Draw a triangle in the current pen colour
   *
   * \param[in] p1 First `Point` of triangle.
   * \param[in] p2 Second `Point` of triangle.
   * \param[in] p3 This `Point` of triangle.
   */
  void Surface::triangle(Point p1, Point p2, Point p3) {
    Rect bounds(
      Point(std::min(p1.x, std::min(p2.x, p3.x)), std::min(p1.y, std::min(p2.y, p3.y))),
      Point(std::max(p1.x, std::max(p2.x, p3.x)), std::max(p1.y, std::max(p2.y, p3.y))));

    // clip extremes to frame buffer size
    Rect mclip = clip; mclip.w--; mclip.h--;
    bounds = mclip.intersection(bounds);

    // if triangle completely out of bounds then don't bother!
    if (bounds.empty()) {
      return;
    }

    // fix "winding" of vertices if needed
    int32_t winding = orient2d(p1, p2, p3);
    if (winding < 0) {
      Point t;
      t = p1; p1 = p3; p3 = t;
    }

    // bias ensures no overdraw between neighbouring triangles
    int8_t bias0 = is_top_left(p2, p3) ? 0 : -1;
    int8_t bias1 = is_top_left(p3, p1) ? 0 : -1;
    int8_t bias2 = is_top_left(p1, p2) ? 0 : -1;

    int32_t a01 = p1.y - p2.y;
    int32_t b01 = p2.x - p1.x;
    int32_t a12 = p2.y - p3.y;
    int32_t b12 = p3.x - p2.x;
    int32_t a20 = p3.y - p1.y;
    int32_t b20 = p1.x - p3.x;

    Point tl(bounds.x, bounds.y);
    int32_t w0row = orient2d(p2, p3, tl) + bias0;
    int32_t w1row = orient2d(p3, p1, tl) + bias1;
    int32_t w2row = orient2d(p1, p2, tl) + bias2;

    Point p;

    for (p.y = bounds.y; p.y <= bounds.y + bounds.h; p.y++) {
      int32_t w0 = w0row;
      int32_t w1 = w1row;
      int32_t w2 = w2row;

      for (p.x = bounds.x; p.x <= bounds.x + bounds.w; p.x++) {
        if ((w0 | w1 | w2) >= 0) {
          pbf(&pen, this, offset(p), 1);
        }

        w0 += a12;
        w1 += a20;
        w2 += a01;
      }

      w0row += b12;
      w1row += b20;
      w2row += b01;
    }
  }

  /**
   * Draw a polygon from a std::vector<point> list of points.
   *
   * \param[in] points `std::vector<point>` of points describing the polygon.
   */
  void Surface::polygon(std::vector<Point> points) {
    static int32_t nodes[64]; // maximum allowed number of nodes per scanline for polygon rendering

    int32_t miny = points[0].y, maxy = points[0].y;

    for (uint16_t i = 1; i < points.size(); i++) {
      miny = std::min(miny, points[i].y);
      maxy = std::max(maxy, points[i].y);
    }

    // for each scanline within the polygon bounds (clipped to clip rect)
    Point p;

    for (p.y = std::max(clip.y, miny); p.y <= std::min(clip.y + clip.h, maxy); p.y++) {
      uint8_t n = 0;
      for (uint16_t i = 0; i < points.size(); i++) {
        uint16_t j = (i + 1) % points.size();
        int32_t sy = points[i].y;
        int32_t ey = points[j].y;
        int32_t fy = p.y;
        if ((sy < fy && ey >= fy) || (ey < fy && sy >= fy)) {
          int32_t sx = points[i].x;
          int32_t ex = points[j].x;
          int32_t px = int32_t(sx + float(fy - sy) / float(ey - sy) * float(ex - sx));

          nodes[n++] = px < clip.x ? clip.x : (px >= clip.x + clip.w ? clip.x + clip.w - 1 : px);// clamp(int32_t(sx + float(fy - sy) / float(ey - sy) * float(ex - sx)), clip.x, clip.x + clip.w);
        }
      }

      uint16_t i = 0;
      while (i < n - 1) {
        if (nodes[i] > nodes[i + 1]) {
          int32_t s = nodes[i]; nodes[i] = nodes[i + 1]; nodes[i + 1] = s;
          if (i) i--;
        }
        else {
          i++;
        }
      }

      for (uint16_t i = 0; i < n; i += 2) {
        h_span(Point(nodes[i], p.y), nodes[i + 1] - nodes[i] + 1);
      }
    }
  }
}

  /*
  void texture_triangle(int32_t x1, int32_t y1, int32_t u1, int32_t v1, int32_t x2, int32_t y2, int32_t u2, int32_t v2, int32_t x3, int32_t y3, int32_t u3, int32_t v3) {
    int32_t minx = min<int32_t>(x1, min<int32_t>(x2, x3));
    int32_t maxx = max<int32_t>(x1, max<int32_t>(x2, x3));
    int32_t miny = min<int32_t>(y1, min<int32_t>(y2, y3));
    int32_t maxy = max<int32_t>(y1, max<int32_t>(y2, y3));

    // if triangle completely out of bounds then don't bother!
    if (maxx < clip.l || minx >= clip.r || maxy < clip.t || miny >= clip.b) {
      return;
    }

    // clip extremes to frame buffer size
    minx = max<int32_t>(minx, clip.l);
    maxx = min<int32_t>(maxx, clip.r - 1);
    miny = max<int32_t>(miny, clip.t);
    maxy = min<int32_t>(maxy, clip.b - 1);

    // bias ensures no overdraw between neighbouring triangles
    int8_t bias0 = is_top_left(x2, y2, x3, y3) ? 0 : -1;
    int8_t bias1 = is_top_left(x3, y3, x1, y1) ? 0 : -1;
    int8_t bias2 = is_top_left(x1, y1, x2, y2) ? 0 : -1;

    int32_t a01 = y1 - y2;
    int32_t b01 = x2 - x1;
    int32_t a12 = y2 - y3;
    int32_t b12 = x3 - x2;
    int32_t a20 = y3 - y1;
    int32_t b20 = x1 - x3;

    int32_t w0row = orient2d(x2, y2, x3, y3, minx, miny) + bias0;
    int32_t w1row = orient2d(x3, y3, x1, y1, minx, miny) + bias1;
    int32_t w2row = orient2d(x1, y1, x2, y2, minx, miny) + bias2;

    for (int32_t y = miny; y <= maxy; y++) {
      int32_t w0 = w0row;
      int32_t w1 = w1row;
      int32_t w2 = w2row;

      for (int32_t x = minx; x <= maxx; x++) {
        if ((w0 | w1 | w2) >= 0) {
          float d = float((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
          float v1w = float((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / d;
          float v2w = float((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / d;
          float v3w = 1.0f - v1w - v2w;

          int32_t tu = (v1w * float(u1) + v2w * float(u2) + v3w * float(u3));
          int32_t tv = (v1w * float(v1) + v2w * float(v2) + v3w * float(v3));
          tu %= 128;
          tv %= 128;

          rgba *t = &sprites[tu + (tv * 128)];

          blend_func((uint8_t *)t, (uint8_t *)target, x + (y * 160));
        }

        w0 += a12;
        w1 += a20;
        w2 += a01;
      }

      w0row += b12;
      w1row += b20;
      w2row += b01;
    }
  }

  //
 // Draw a circle.
  //
  // Implemented with the mid-point circle algorithm.
 //
  // \param[in] {point} c Center `point` of the circle.
 // \param[in] {int32_t} r Radius of the circle in pixels.
 //
  void outline_circle(const point &c, int32_t r) {
    // if circle completely out of bounds then don't bother!
    if (!clip.intersects(rect(c.x - r, c.y - r, r * 2, r * 2))) {
      return;
    }

    int x = r - 1, y = 0, dx = 1, dy = 1, err = dx - (r << 1);
    while (x >= y)
    {
      pixel(point(c.x + x, c.y + y)); pixel(point(c.x + y, c.y + x));
      pixel(point(c.x - y, c.y + x)); pixel(point(c.x - x, c.y + y));
      pixel(point(c.x - x, c.y - y)); pixel(point(c.x - y, c.y - x));
      pixel(point(c.x + y, c.y - x)); pixel(point(c.x + x, c.y - y));

      if (err <= 0) { y++; err += dy; dy += 2; }
      if (err > 0) { x--; dx += 2; err += dx - (r << 1); }
    }
  }
}

*/
#include <algorithm>
#include "renderer.hpp"

using namespace std;
using namespace blit;

int32_t edge_function(const Point& p1, const Point& p2, const Point& p3) {
  return ((p2.x - p1.x) * (p3.y - p1.y)) - ((p2.y - p1.y) * (p3.x - p1.x));
}

bool is_top_left(const Point& p1, const Point& p2) {
  return (p1.y == p2.y && p1.x > p2.x) || (p1.y < p2.y);
}

int32_t modulo(int32_t x, int32_t n) {
  return (x % n + n) % n;
}

uint32_t pixels_drawn = 0;

void draw_face(Vec3* vertices, Vec3* normals, Vec2* texture_coordinates, Surface* texture, Vec3 light, Pen *color, int16_t* zbuffer, int16_t near, int16_t far) {
  // convert vertices into Q8 integer points
  Point p0(vertices[0].x, vertices[0].y);
  Point p1(vertices[1].x, vertices[1].y);
  Point p2(vertices[2].x, vertices[2].y);

  // back-face cull if winding order is reversed
  int32_t area = edge_function(p0, p1, p2);
  if (area <= 0) { return; }

  // calculate bounds clipped to screen  
  int32_t minx = max(screen.clip.x, min(p0.x, min(p1.x, p2.x)));
  int32_t miny = max(screen.clip.y, min(p0.y, min(p1.y, p2.y)));
  int32_t maxx = min(screen.clip.x + screen.clip.w - 1, max(p0.x, max(p1.x, p2.x)));
  int32_t maxy = min(screen.clip.y + screen.clip.h - 1, max(p0.y, max(p1.y, p2.y)));

  // nothing to draw
  if (minx == maxx && miny == maxy) { return; }

  // bias ensures no overdraw between neighbouring triangles along the shared edge
  // now it's fixed point can this just always be applied as -128?
  int32_t bias0 = is_top_left(p1, p2) ? 0 : -1;
  int32_t bias1 = is_top_left(p2, p0) ? 0 : -1;
  int32_t bias2 = is_top_left(p0, p1) ? 0 : -1;

  Point p(minx, miny);
  int32_t a01 = p0.y - p1.y, b01 = p1.x - p0.x;
  int32_t a12 = p1.y - p2.y, b12 = p2.x - p1.x;
  int32_t a20 = p2.y - p0.y, b20 = p0.x - p2.x;
  int32_t w0row = edge_function(p1, p2, p) - bias0;
  int32_t w1row = edge_function(p2, p0, p) - bias1;
  int32_t w2row = edge_function(p0, p1, p) - bias2;

  float invarea = 1.0f / area;

  float t1x, t1y, t2x, t2y, t3x, t3y;

  if (texture) {
    t1x = texture_coordinates[0].x * texture->bounds.w;
    t1y = texture_coordinates[0].y * texture->bounds.h;
    t2x = texture_coordinates[1].x * texture->bounds.w;
    t2y = texture_coordinates[1].y * texture->bounds.h;
    t3x = texture_coordinates[2].x * texture->bounds.w;
    t3y = texture_coordinates[2].y * texture->bounds.h;
  }

  for (p.y = miny; p.y < maxy; p.y++) {
    uint32_t pixel_offset = screen.offset(minx, p.y);

    int32_t w0 = w0row;
    int32_t w1 = w1row;
    int32_t w2 = w2row;

    for (p.x = minx; p.x < maxx; p.x++) {
      // check if sign bit is set in any of our barycentric coordinates
      // if so we're outside of the triangle so skip the pixel
      if ((w0 | w1 | w2) >= 0) {

        // calculate barycentric coordinates of current pixel
        float alpha = w0 * invarea;
        float  beta = w1 * invarea;
        float gamma = 1.0f - alpha - beta; // ensures sum == 1.0f

        int16_t z = (vertices[0].z * alpha + vertices[1].z * beta + vertices[2].z * gamma) * 16384;

        if (z > zbuffer[p.x + (p.y * screen.bounds.w)] && z > -1) {
          zbuffer[p.x + (p.y * screen.bounds.w)] = z;

          pixels_drawn++;

          if (texture) {
            Point interpolated_uv(
              t1x * alpha + t2x * beta + t3x * gamma,
              t1y * alpha + t2y * beta + t3y * gamma
            );

            interpolated_uv.x = modulo(interpolated_uv.x, texture->bounds.w);
            interpolated_uv.y = texture->bounds.h - modulo(interpolated_uv.y, texture->bounds.h);
            screen.pen = texture->palette[texture->data[interpolated_uv.x + interpolated_uv.y * texture->bounds.w]];
          } else {
            screen.pen = *color;
          }

          Vec3 interpolated_n = normals[0] * alpha + normals[1] * beta + normals[2] * gamma;
          float diffuse = interpolated_n.dot(light);            
          diffuse = std::max(diffuse, 0.2f);
          screen.pen.r *= diffuse;
          screen.pen.g *= diffuse;
          screen.pen.b *= diffuse;

          screen.pbf(&screen.pen, &screen, pixel_offset, 1);
        }
      }

      w0 += a12;
      w1 += a20;
      w2 += a01;
      pixel_offset++;
    }

    w0row += b12;
    w1row += b20;
    w2row += b01;
  }
}

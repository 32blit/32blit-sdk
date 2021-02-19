
#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "32blit.hpp"
#include "3d/camera.hpp"
#include "object.hpp"
#include "renderer.hpp"
#include "link.hpp"

using namespace blit;
using namespace std;

SpriteSheet *main_texture;
SpriteSheet *boots_texture;
SpriteSheet *eye_texture;
SpriteSheet *mouth_texture;
SpriteSheet *handopen_texture;
SpriteSheet *bracelet_texture;
SpriteSheet *glove_texture;
SpriteSheet *sheath_texture;
SpriteSheet *bookshelf_texture;

float* zbuffer;

object *link_object;
Camera cam(
  Vec3(0.0f, 0.0f, -1.5f), 
  Vec3(0.0f, 0.0f, -1.0f),
  Vec3(0.0f, 1.0f, 0.0f),
  90.0f);

float scale = 1.0f;

constexpr uint32_t fixed_shift = 0;
constexpr uint32_t fixed_mul = 1 << fixed_shift;

void init() {
  set_screen_mode(lores);

  zbuffer = new float[screen.bounds.w * screen.bounds.h];
  
  /*
  link_object = load_obj((char*)link_obj);    
  main_texture = SpriteSheet::load(main_texture_packed);
  boots_texture = SpriteSheet::load(boots_texture_packed);
  eye_texture = SpriteSheet::load(eye_texture_packed);
  mouth_texture = SpriteSheet::load(mouth_texture_packed);
  handopen_texture = SpriteSheet::load(handopen_texture_packed);
  bracelet_texture = SpriteSheet::load(bracelet_texture_packed);
  glove_texture = SpriteSheet::load(glove_texture_packed);
  sheath_texture = SpriteSheet::load(sheath_texture_packed);
  link_object->g[1].t = main_texture;
  link_object->g[2].t = main_texture;
  link_object->g[3].t = boots_texture;
  link_object->g[4].t = eye_texture;
  link_object->g[5].t = mouth_texture;
  link_object->g[6].t = handopen_texture;
  link_object->g[7].t = bracelet_texture;
  link_object->g[8].t = glove_texture;
  link_object->g[9].t = sheath_texture;*/

  link_object = load_obj((char*)bookshelf_obj);
  bookshelf_texture = SpriteSheet::load(bookshelf_texture_packed);
  //link_object->g[0].t = bookshelf_texture;

  for (uint32_t gi = 0; gi < link_object->gc; gi++) {
    group* g = &link_object->g[gi];
    for (uint32_t fi = 0; fi < g->fc; fi++) {
      // sample texture for face color
      face* f = &g->f[fi];
      Vec2* uv = &link_object->t[f->t[0]];
      uint32_t u = uv->x * bookshelf_texture->bounds.w;
      uint32_t v = bookshelf_texture->bounds.h - (uv->y * bookshelf_texture->bounds.h);

      uint8_t pi = bookshelf_texture->data[u + (v * bookshelf_texture->bounds.w)];
      f->color = bookshelf_texture->palette[pi];
    }
  }

}

void debug_matrix(Mat4& t) {
  screen.pen = Pen(255, 255, 255);
  screen.text(std::to_string(int(t.v00 * 1000)), minimal_font, Point(0, 100));
  screen.text(std::to_string(int(t.v10 * 1000)), minimal_font, Point(0, 110));
  screen.text(std::to_string(int(t.v20 * 1000)), minimal_font, Point(0, 120));
  screen.text(std::to_string(int(t.v30 * 1000)), minimal_font, Point(0, 130));

  screen.text(std::to_string(int(t.v01 * 1000)), minimal_font, Point(30, 100));
  screen.text(std::to_string(int(t.v11 * 1000)), minimal_font, Point(30, 110));
  screen.text(std::to_string(int(t.v21 * 1000)), minimal_font, Point(30, 120));
  screen.text(std::to_string(int(t.v31 * 1000)), minimal_font, Point(30, 130));

  screen.text(std::to_string(int(t.v02 * 1000)), minimal_font, Point(60, 100));
  screen.text(std::to_string(int(t.v12 * 1000)), minimal_font, Point(60, 110));
  screen.text(std::to_string(int(t.v22 * 1000)), minimal_font, Point(60, 120));
  screen.text(std::to_string(int(t.v32 * 1000)), minimal_font, Point(60, 130));

  screen.text(std::to_string(int(t.v03 * 1000)), minimal_font, Point(90, 100));
  screen.text(std::to_string(int(t.v13 * 1000)), minimal_font, Point(90, 110));
  screen.text(std::to_string(int(t.v23 * 1000)), minimal_font, Point(90, 120));
  screen.text(std::to_string(int(t.v33 * 1000)), minimal_font, Point(90, 130));

  return;
}

float near = 1.0f;
float far = 50.0f;

void render(uint32_t time_ms) {
  // clear the screen buffer
  screen.pen = Pen(20, 30, 40);
  screen.clear();

  // reset the zbuffer
  for (uint32_t i = 0; i < screen.bounds.w * screen.bounds.h; i++) {
    zbuffer[i] = -1.0f;
  }

  uint32_t ms_start = now();


  Vec3 light(1.0f, 0.0f, 0.0f);

  // link object 

  // world to screen transformation matrix
  float orthographic_scale = 1.0f / 80.0f; // 80 pixels per world unit
  Mat4 camera_transformation = Mat4::identity();      
  // centre to screen viewport
  camera_transformation *= Mat4::translation(Vec3(screen.bounds.w / 2.0f, screen.bounds.h / 2.0f, 0.0f)); 
  // scale to screen viewport
  camera_transformation *= Mat4::scale(Vec3(screen.bounds.w, screen.bounds.w, 1.0f));
  // camera projection  
  //camera_transformation *= cam.ortho_projection_matrix(1.0f, 1.0f);
  camera_transformation *= cam.perspective_projection_matrix(screen.clip, near, far);
  // camera direction
  camera_transformation *= cam.rotation_matrix();
  // camera position
  camera_transformation *= cam.translation_matrix();

  // object transformation matrix
  Mat4 object_transformation = Mat4::identity();
  object_transformation *= Mat4::translation(Vec3(0.0f, 0.0f, -3.0f));
  // rotate around y axis based on time
  object_transformation *= Mat4::rotation(time_ms / 20.0f, Vec3(0.0f, 1.0f, 0.0f));
  // scale link to be 1 world unit wide
  float link_scale = 1.0f / (link_object->bounds.v2.x - link_object->bounds.v1.x);
  //link_scale *= 10.0f;
  object_transformation *= Mat4::scale(Vec3(link_scale, link_scale, link_scale));
  // links feet are anchored at y = 0 so let's offset him by half his height
  // because it's easier to position him for testing when 0, 0, 0 is bang in the centre
  float y_offset = (link_object->bounds.v2.y - link_object->bounds.v1.y) / 2.0f;
  object_transformation *= Mat4::translation(Vec3(0.0f, -y_offset, 0.0f));

  // combine transformations
  Mat4 transformation = camera_transformation * object_transformation;


  // apply the inverse object transform to the light source
  // this is effectively the same as transforming all of the objects
  // normals but a lot faster!
  Mat4 inverse_object_transformation = object_transformation;
  inverse_object_transformation.inverse();

  Vec3 transformed_light = inverse_object_transformation.transform(light);
  transformed_light.normalize();

  uint32_t tri_count = 0;
  pixels_drawn = 0;

  for (uint32_t gi = 0; gi < link_object->gc; gi++) {
    if (gi == 8) { // hide links "second" pair of hands...
      continue;
    }
    group *g = &link_object->g[gi];
    for (uint32_t fi = 0; fi < g->fc; fi++) {
      face *f = &g->f[fi];        
 
      Vec3 vertices[3] = {
        transformation.transform(link_object->v[f->v[0]]),
        transformation.transform(link_object->v[f->v[1]]),
        transformation.transform(link_object->v[f->v[2]])
      };

      Vec2 texture_coordinates[3] = {
        link_object->t[f->t[0]],
        link_object->t[f->t[1]],
        link_object->t[f->t[2]]
      };

      Vec3 normals[3] = {
        link_object->n[f->n[0]],
        link_object->n[f->n[1]],
        link_object->n[f->n[2]]
      };
        
     // if (g->t) {
        //triangle3d(&screen, link_object, f, v, &objlight, g->t/*, st, &shadowmap*/);
        //screen.pen = Pen(255, 255, 255);
        draw_face(vertices, normals, texture_coordinates, g->t, transformed_light, &f->color, zbuffer, near, far);
    
        /*screen.pen = Pen(255, 255, 255, 100);
        screen.line(Point(vertices[0].x, vertices[0].y), Point(vertices[1].x, vertices[1].y));
        screen.line(Point(vertices[1].x, vertices[1].y), Point(vertices[2].x, vertices[2].y));
        screen.line(Point(vertices[2].x, vertices[2].y), Point(vertices[0].x, vertices[0].y));
*/
       /* screen.pixel(Point(v[0].x, v[0].y));
        screen.pixel(Point(v[1].x, v[1].y));
        screen.pixel(Point(v[2].x, v[2].y));*/
        tri_count++;
      //}
    }
  }
  
  
  
  uint32_t ms_end = now();  
  
  screen.pen = Pen(255, 255, 255);  
  screen.text(std::to_string(tri_count), minimal_font, Rect(2, screen.bounds.h - 33, 50, 10));
  screen.text(std::to_string(pixels_drawn), minimal_font, Rect(2, screen.bounds.h - 23, 50, 10));


  // draw FPS meter* 
  screen.pen = Pen(255, 255, 255);
  std::string fms = std::to_string(ms_end - ms_start);
  screen.text(fms, minimal_font, Rect(2, screen.bounds.h - 13, 50, 10));

  int block_size = 4;
  for (uint32_t i = 0; i < (ms_end - ms_start); i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(Rect(i * (block_size + 1) + 1, screen.bounds.h - block_size - 1, block_size, block_size));
  }

}

void update(uint32_t time) {
  static uint32_t last_buttons = 0;

  if (buttons != last_buttons) {  
/*    if ((buttons & DPAD_UP)) {
      set_screen_mode(lores);
      mask = lores_mask;
      screen.sprites = ss;
    }
    else {
      set_screen_mode(hires);
      mask = hires_mask;
      screen.sprites = ss;
    }*/
  }

  cam.position += cam.direction * joystick.y * 0.1f;

  last_buttons = buttons;
}


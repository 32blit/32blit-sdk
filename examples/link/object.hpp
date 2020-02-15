#include "32blit.hpp"

using namespace blit;

struct bbox {
  Vec3 v1, v2;          // opposing (min / max) corners of bounding box
};

struct face {
  Pen color;
  uint32_t v[3];  // vertex indices
  uint32_t t[3];  // texture indices
  uint32_t n[3];  // normal indices
};

class texture {
public:
  uint32_t width;
  uint32_t height;
  Pen* data;

  ~texture();
};

class group {
public:
  group();
  ~group();

  face     *f;            // pointer to face list
  uint32_t  fc = 0;           // face count

  Surface * t = NULL;
};


class object {
public:
  Vec3   *v;            // pointer to vertex list
  uint32_t  vc = 0;           // vertex count
  Vec2   *t;            // pointer to texture coordinate list
  uint32_t  tc = 0;           // texture coordinate count
  Vec3   *n;            // pointer to normal Vec3s list
  uint32_t  nc = 0;           // texture coordinate count

  group    *g;            // pointer to group list
  uint32_t  gc = 0;           // group count

  bbox      bounds;       // bounding box

  Mat4      m;            // transformation matrix    

  // !! could be nice to have the option to point to a shader program?
  // !! if you have a Pen shader then you really need a depth shader too?

  object();
  //object(vertex *v, uint32_t vc, face *f, uint32_t fc, Vec3 *t, uint32_t tc);
  ~object();

  void update_bbox();
};




void triangle3d(Surface *fb, object *obj, face *f, Vec3 *p, Vec3 *l, texture *t);
void triangle3d_span(Surface *fb, object *obj, face *f, int32_t y, Vec3 *p, uint8_t a, uint8_t b, uint8_t c, uint8_t d, Vec3 *l, texture* t);


object *load_obj(char *data);

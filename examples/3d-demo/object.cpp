#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <algorithm>

#include "object.hpp"

using namespace blit;
using namespace std;


Object::Object() {
}

//  Object::Object(vertex *v, uint32_t vc, face *f, uint32_t fc, Vec3 *t, uint32_t tc) :v(v), vc(vc), f(f), fc(fc), t(t), tc(tc) {
//    update_bbox();
//  }

Object::~Object() {
  delete[] v;
  delete[] t;
  delete[] n;
  delete[] g;
}

void Object::update_bbox() {
  bounds.v1.x = v[0].x;
  bounds.v1.y = v[0].y;
  bounds.v1.z = v[0].z;
  bounds.v2.x = v[0].x;
  bounds.v2.y = v[0].y;
  bounds.v2.z = v[0].z;

  for (uint32_t i = 0; i < vc; i++) {
    bounds.v1.x = min(v[i].x, bounds.v1.x);
    bounds.v1.y = min(v[i].y, bounds.v1.y);
    bounds.v1.z = min(v[i].z, bounds.v1.z);
    bounds.v2.x = max(v[i].x, bounds.v2.x);
    bounds.v2.y = max(v[i].y, bounds.v2.y);
    bounds.v2.z = max(v[i].z, bounds.v2.z);
  }
}

Group::Group() {

}

Group::~Group() {
  delete[] f;
  delete[] t;
}
/* 
  camera
*/



void discard_whitespace(char** p, char* eof, char extra_delimiter = '\0') {
  while ((**p == ' ' || **p == '\t' || **p == '\n' || **p == extra_delimiter) && (*p < eof)) {
    (*p)++;
  }
}

const char* fetch_token(char** p, char* eof, char extra_delimiter = '\0') {
  static char token[32];
  memset(token, 0, 32);

  char* t = token;
  while ((**p != '\t') && (**p != ' ') && (**p != '\n') && (**p != extra_delimiter) && (*p < eof)) {
    *t = **p;
    (*p)++;
    t++;
  }

  discard_whitespace(p, eof, extra_delimiter);

  return token;
}

Object* Object::load_obj(char *data) {
  char *p = data;
  char *eof = p + strlen(data);

  Object *obj = new Object();

  // first pass: count number of groups, vertices, texture coords, and normal Vec3s
  while (p < eof) { 
    const char *token = fetch_token(&p, eof);

    if(!strcmp(token, "g")) {
      obj->gc++;
    }
    
    if(!strcmp(token, "v")) {
      obj->vc++;
    }

    if(!strcmp(token, "vn")) {
      obj->nc++;
    }
    
    if(!strcmp(token, "vt")) {
      obj->tc++;
    }        
  }

  // allocate storage
  obj->g = new Group[obj->gc];
  obj->v = new Vec3[obj->vc];
  obj->t = new Vec2[obj->tc];
  obj->n = new Vec3[obj->nc];

  // group index starts at -1 since it gets incremented each time we encounter a
  // group ("g") command.
  uint32_t gi = -1, vi = 0, ti = 0, ni = 0;

  // second pass: extra vertices, texture coordinates, and normals
  // also count the number of faces per group
  p = data; // back to the start...
  while (p < eof) {
    const char* token = fetch_token(&p, eof);

    // each time we encounter a group ("g") then we move on to the next group index
    // to start counting the faces for that group
    if (!strcmp(token, "g")) {
      gi++;
    }

    if (!strcmp(token, "f")) {
      // if it's a quad then we need to add an extra face
      uint8_t space_count = 0;
      while (*p != '\n') {
        if (*p == ' ') {
          space_count++;
        }
        p++;
      }
      // grotty hack to ignore trailing space - needs to be handled properly
      if (*(p - 1) == ' ') {
        space_count--;
      }
      obj->g[gi].fc += space_count < 3 ? 1 : 2;
    }

    if (!strcmp(token, "v")) {
      obj->v[vi].x = atof(fetch_token(&p, eof));
      obj->v[vi].y = atof(fetch_token(&p, eof));
      obj->v[vi].z = atof(fetch_token(&p, eof));
      vi++;
    }

    if (!strcmp(token, "vn")) {
      obj->n[ni].x = atof(fetch_token(&p, eof));
      obj->n[ni].y = atof(fetch_token(&p, eof));
      obj->n[ni].z = atof(fetch_token(&p, eof));
      ni++;
    }
    
    if (!strcmp(token, "vt")) {
      obj->t[ti].x = atof(fetch_token(&p, eof));
      obj->t[ti].y = atof(fetch_token(&p, eof));
      ti++;
    }
    
  }
  
  // allocate storage for faces on each group
  for (gi = 0; gi < obj->gc; gi++) {
    obj->g[gi].f = new Face[obj->g[gi].fc];
  }

  // third pass: now extract the face indices
  uint32_t fi = 0; gi = -1;
  p = data; // back to the start...
  while (p < eof) {
    const char *token = fetch_token(&p, eof);

    if (!strcmp(token, "g")) {
        gi++; fi = 0;
    }

    if (!strcmp(token, "f")) {
      Face *f = &obj->g[gi].f[fi];

      // read in this order (2, 1, 0) to reverse the default 
      // "counter clockwise" winding order in .obj files
      f->v[2] = atol(fetch_token(&p, eof, '/')) - 1;
      f->t[2] = atol(fetch_token(&p, eof, '/')) - 1;
      f->n[2] = atol(fetch_token(&p, eof, '/')) - 1;  // TODO: support files without face normals

      f->v[1] = atol(fetch_token(&p, eof, '/')) - 1;
      f->t[1] = atol(fetch_token(&p, eof, '/')) - 1;
      f->n[1] = atol(fetch_token(&p, eof, '/')) - 1;  // TODO: support files without face normals

      f->v[0] = atol(fetch_token(&p, eof, '/')) - 1;
      f->t[0] = atol(fetch_token(&p, eof, '/')) - 1;
      f->n[0] = atol(fetch_token(&p, eof, '/')) - 1;  // TODO: support files without face normals

      // if there is another vertex then it must be a quad so add the second face
      if (*(p - 1) != '\n') {
        fi++;

        Face *lf = f;
        Face *f = &obj->g[gi].f[fi];

        // read in this order (2, 1, 0) to reverse the default 
        // "counter clockwise" winding order in .obj files
        f->v[2] = atol(fetch_token(&p, eof, '/')) - 1;
        f->t[2] = atol(fetch_token(&p, eof, '/')) - 1;
        f->n[2] = atol(fetch_token(&p, eof, '/')) - 1;  // TODO: support files without face normals

        f->v[1] = lf->v[2];
        f->t[1] = lf->t[2];
        f->n[1] = lf->n[2];

        f->v[0] = lf->v[0];
        f->t[0] = lf->t[0];
        f->n[0] = lf->n[0];
      }

      fi++;
    }
  }

  obj->update_bbox();

  return obj;
}
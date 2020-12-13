/*! \file vec2.cpp
*/
#include <cstring>

#include "vec2.hpp"
#include "mat3.hpp"

namespace blit {

  void Vec2::rotate(float a) {
    float c = cosf(a);
    float s = sinf(a);
    float rx = this->x * c - this->y * s;
    float ry = this->x * s + this->y * c;
    this->x = rx;
    this->y = ry;
  }


  void Vec2::transform(const Mat3 &m) {
    float tx = x; float ty = y;
    this->x = (m.v00 * tx + m.v01 * ty + m.v02);
    this->y = (m.v10 * tx + m.v11 * ty + m.v12);
  }

  float Vec2::angle(Vec2 o) {
    //atan2d(x1*y2-y1*x2,x1*x2+y1*y2);

    return std::atan2(this->x * o.y - this->y * o.x, this->x * o.x + this->y * o.y);
    //return acos(this->dot(o));
  }

  /*
    creates a new vec2 userdata and pushes it onto the Lua stack
  */
  /*
  vec2 *push_vec2(lua_State* L, float x, float y) {
    vec2 *v = (vec2 *)lua_newuserdata(L, sizeof(vec2));
    luaL_getmetatable(L, "vec2");
    lua_setmetatable(L, -2);
    v->x = x; v->y = y;
    return v;
  }

  vec2 *push_vec2(lua_State* L, const vec2 &v) {
    return push_vec2(L, v.x, v.y);
  }

  extern "C" {

    static int vec2_new(lua_State* L) {
      if (!lua_isnumber(L, 1)) {
        // copy constructor
        vec2 *src = (vec2 *)luaL_checkudata(L, 1, "vec2");
        push_vec2(L, src->x, src->y);
      }
      else {
        // standard constructor
        push_vec2(L,
          luaL_optnumber(L, 1, 0),
          luaL_optnumber(L, 2, 0));
      }
      return 1;
    }

    static int vec2_normalize(lua_State* L) {
      vec2 *v = (vec2 *)luaL_checkudata(L, 1, "vec2");

      v->normalize();

      lua_settop(L, 1);

      return 1;
    }

    static int vec2_rotate(lua_State* L) {
      vec2 *v = (vec2 *)luaL_checkudata(L, 1, "vec2");
      float a = (float)luaL_checknumber(L, 2);

      v->rotate(a);

      lua_settop(L, 1);

      return 1;
    }

    static int vec2_reflect(lua_State* L) {
      vec2 *iv = (vec2 *)luaL_checkudata(L, 1, "vec2");
      vec2 *nv = (vec2 *)luaL_checkudata(L, 2, "vec2");

      float m = 2.0f * iv->dot(nv);

      push_vec2(L, *iv - *nv * m);

      return 1;
    }

    static int vec2_refract(lua_State* L) {
      vec2 *iv = (vec2 *)luaL_checkudata(L, 1, "vec2");
      vec2 *nv = (vec2 *)luaL_checkudata(L, 2, "vec2");
      float ri = (float)luaL_checknumber(L, 3);

      float ni = nv->dot(iv);
      float k = 1.0f - ri * ri * (1.0f - ni * ni);
      if (k < 0.0f) {
        push_vec2(L, 0.0f, 0.0f);
      }
      else {
        vec2 rv = *iv * ri - *nv * (ri * ni + sqrtf(k));
        push_vec2(L, rv);
      }

      return 1;
    }

    static int vec2_dot(lua_State* L) {
      vec2 *v1 = (vec2 *)luaL_checkudata(L, 1, "vec2");
      vec2 *v2 = (vec2 *)luaL_checkudata(L, 2, "vec2");

      lua_pushnumber(L, v1->dot(v2));

      return 1;
    }

    static int vec2_length(lua_State* L) {
      vec2 *v = (vec2 *)luaL_checkudata(L, 1, "vec2");

      lua_pushnumber(L, v->length());

      return 1;
    }

    // operators
    static int vec2__add(lua_State* L) {
      vec2 *v1 = (vec2 *)luaL_checkudata(L, 1, "vec2");
      vec2 *v2 = (vec2 *)luaL_checkudata(L, 2, "vec2");

      push_vec2(L, *v1 + *v2);

      return 1;
    }

    static int vec2__sub(lua_State* L) {
      vec2 *v1 = (vec2 *)luaL_checkudata(L, 1, "vec2");
      vec2 *v2 = (vec2 *)luaL_checkudata(L, 2, "vec2");

      push_vec2(L, *v1 - *v2);

      return 1;
    }

    static int vec2__mul(lua_State* L) {
      vec2 *v;
      float m;

      if (lua_isnumber(L, 1)) {
        // m * v
        m = (float)luaL_checknumber(L, 1);
        v = (vec2 *)luaL_checkudata(L, 2, "vec2");
      }
      else {
        // v * m
        v = (vec2 *)luaL_checkudata(L, 1, "vec2");
        m = (float)luaL_checknumber(L, 2);
      }

      push_vec2(L, *v * m);

      return 1;
    }

    static int vec2__unm(lua_State* L) {
      vec2 *v = (vec2 *)luaL_checkudata(L, 1, "vec2");

      push_vec2(L, -(*v));

      return 1;
    }

    static int vec2__index(lua_State* L) {
      vec2 *v = (vec2 *)luaL_checkudata(L, 1, "vec2");

      // some magic to call the correct method
      luaL_getmetatable(L, "vec2");
      lua_pushvalue(L, 2);
      lua_rawget(L, -2);

      // if it wasn't a method then do a property access instead
      if (lua_isnil(L, -1)) {
        const char *n = luaL_checkstring(L, 2);

        if (!strcmp(n, "x")) {
          lua_pushnumber(L, v->x);
          return 1;
        }
        if (!strcmp(n, "y")) {
          lua_pushnumber(L, v->y);
          return 1;
        }
      }

      return 1;
    }

    static int vec2__newindex(lua_State* L) {
      vec2 *v = (vec2 *)luaL_checkudata(L, 1, "vec2");

      const char *n = luaL_checkstring(L, 2);

      if (!strcmp(n, "x")) {
        v->x = lua_tonumber(L, 3);
      }
      if (!strcmp(n, "y")) {
        v->y = lua_tonumber(L, 3);
      }

      return 0;
    }
  }

  void register_vec2lib(lua_State *L) {
    // constructor
    lua_register(L, "vec2", vec2_new);

    luaL_newmetatable(L, "vec2");

    // push static methods

    // push meta methods
    lua_pushvalue(L, -1); lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, vec2__index); lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, vec2__add); lua_setfield(L, -2, "__add");
    lua_pushcfunction(L, vec2__unm); lua_setfield(L, -2, "__unm");
    lua_pushcfunction(L, vec2__sub); lua_setfield(L, -2, "__sub");
    lua_pushcfunction(L, vec2__mul); lua_setfield(L, -2, "__mul");
    lua_pushcfunction(L, vec2__newindex); lua_setfield(L, -2, "__newindex");

    // push instance methods
    lua_pushcfunction(L, vec2_normalize); lua_setfield(L, -2, "normalize");
    lua_pushcfunction(L, vec2_rotate); lua_setfield(L, -2, "rotate");
    lua_pushcfunction(L, vec2_dot); lua_setfield(L, -2, "dot");
    lua_pushcfunction(L, vec2_length); lua_setfield(L, -2, "length");
    lua_pushcfunction(L, vec2_reflect); lua_setfield(L, -2, "reflect");
    lua_pushcfunction(L, vec2_refract); lua_setfield(L, -2, "refract");

    lua_pop(L, 1);
  }
  */
}
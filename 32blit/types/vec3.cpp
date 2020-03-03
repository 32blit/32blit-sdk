/*! \file vec3.cpp
*/
#include <cmath>
#include <cstring>

#include "vec3.hpp"
#include "mat4.hpp"
/*
extern "C" {
  #include <lua\lua.h>
  #include <lua\lualib.h>
  #include <lua\lauxlib.h>
}*/

Vec3::Vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}


void Vec3::transform(const Mat4 &m) {
  float w = m.v30 * this->x + m.v31 * this->y + m.v32 * -this->z + m.v33;
  float tx = x; float ty = y; float tz = z;
  this->x = (m.v00 * tx + m.v01 * ty + m.v02 * -tz + m.v03) / w;
  this->y = (m.v10 * tx + m.v11 * ty + m.v12 * -tz + m.v13) / w;
  this->z = (m.v20 * tx + m.v21 * ty + m.v22 * -tz + m.v23) / w;
}

void   Vec3::normalize() { float d = this->length(); x /= d; y /= d; z /= d; }
float  Vec3::length() { return sqrtf(x * x + y * y + z * z); }
Vec3   Vec3::cross(const Vec3 &a) { return Vec3(y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x); }
Vec3   Vec3::cross(const Vec3 *a) { return Vec3(y * a->z - z * a->y, z * a->x - x * a->z, x * a->y - y * a->x); }
float  Vec3::dot(const Vec3 &a) { return (x * a.x) + (y * a.y) + (z * a.z); }
float  Vec3::dot(const Vec3 *a) { return (x * a->x) + (y * a->y) + (z * a->z); }



/*
void register_vec3lib(lua_State *L);
vec3 *push_vec3(lua_State* L, float x = 0, float y = 0, float z = 0);
vec3 *push_vec3(lua_State* L, const vec3 &v);*/



/* 
  creates a new vec3 userdata and pushes it onto the Lua stack
*/

/*
vec3 *push_vec3(lua_State* L, float x, float y, float z) {
  vec3 *v = (vec3 *)lua_newuserdata(L, sizeof(vec3));
  luaL_getmetatable(L, "vec3");
  lua_setmetatable(L, -2);
  v->x = x; v->y = y; v->z = z;
  return v;
}

vec3 *push_vec3(lua_State* L, const vec3 &v) {
  return push_vec3(L, v.x, v.y, v.z);
}

extern "C" {

  static int vec3_new(lua_State* L) {
    if (!lua_isnumber(L, 1)) {
      // copy constructor
      vec3 *src = (vec3 *)luaL_checkudata(L, 1, "vec3");
      push_vec3(L, src->x, src->y, src->z);
    } else {
      // standard constructor
      push_vec3(L,
        luaL_optnumber(L, 1, 0),
        luaL_optnumber(L, 2, 0),
        luaL_optnumber(L, 3, 0));
    }
    return 1;
  }

  static int vec3_normalize(lua_State* L) {
    vec3 *v = (vec3 *)luaL_checkudata(L, 1, "vec3");

    v->normalize();

    lua_settop(L, 1);

    return 1;
  }

  static int vec3_reflect(lua_State* L) {
    vec3 *iv = (vec3 *)luaL_checkudata(L, 1, "vec3");
    vec3 *nv = (vec3 *)luaL_checkudata(L, 2, "vec3");

    float m = 2.0f * iv->dot(nv);
    
    push_vec3(L, *iv - *nv * m);
    
    return 1;
  }

  static int vec3_refract(lua_State* L) {
    vec3 *iv = (vec3 *)luaL_checkudata(L, 1, "vec3");
    vec3 *nv = (vec3 *)luaL_checkudata(L, 2, "vec3");
    float ri = (float)luaL_checknumber(L, 3);

    float m = iv->dot(nv);
    float k = 1.0f - ri * ri * (1.0f - m * m);
    if (k < 0.0f) {
      push_vec3(L, 0.0f, 0.0f, 0.0f);
    } else {
      vec3 *v = push_vec3(L, (*iv) * ri - (*nv) * (ri * m + sqrtf(k)));
    }

    return 1;
  }

  static int vec3_dot(lua_State* L) {
    vec3 *v1 = (vec3 *)luaL_checkudata(L, 1, "vec3");
    vec3 *v2 = (vec3 *)luaL_checkudata(L, 2, "vec3");

    lua_pushnumber(L, v1->dot(v2));

    return 1;
  }

  static int vec3_length(lua_State* L) {
    vec3 *v = (vec3 *)luaL_checkudata(L, 1, "vec3");

    lua_pushnumber(L, v->length());

    return 1;
  }

  // operators
  static int vec3__add(lua_State* L) {
    vec3 *v1 = (vec3 *)luaL_checkudata(L, 1, "vec3");
    vec3 *v2 = (vec3 *)luaL_checkudata(L, 2, "vec3");

    push_vec3(L, *v1 + *v2);
    
    return 1;
  }

  static int vec3__sub(lua_State* L) {
    vec3 *v1 = (vec3 *)luaL_checkudata(L, 1, "vec3");
    vec3 *v2 = (vec3 *)luaL_checkudata(L, 2, "vec3");

    push_vec3(L, *v1 - *v2);

    return 1;
  }

  static int vec3__mul(lua_State* L) {
    vec3 *v;
    float m;

    if (lua_isnumber(L, 1)) {                       
      // m * v
      m = (float)luaL_checknumber(L, 1);
      v = (vec3 *)luaL_checkudata(L, 2, "vec3");      
    } else {                                          
      // v * m
      v = (vec3 *)luaL_checkudata(L, 1, "vec3");
      m = (float)luaL_checknumber(L, 2);
    }

    push_vec3(L, *v * m);

    return 1;
  }

  static int vec3__unm(lua_State* L) {
    vec3 *v = (vec3 *)luaL_checkudata(L, 1, "vec3");

    push_vec3(L, -(*v));

    return 1;
  }




 
  static int vec3__gc(lua_State* L) {
    vec3 *v = (vec3 *)luaL_checkudata(L, 1, "vec3");
    delete v;
    return 0;
  } 

  
  static int vec3__call(lua_State* L) {
    vec3 *v = (vec3 *)luaL_checkudata(L, 1, "vec3");
    const char *n = luaL_checkstring(L, 2);

    if (!strcmp(n, "normalize")) {
      v->normalize();
      lua_settop(L, 1);
      return 1;
    }

    return 0;
  }
  static int vec3__index(lua_State* L) {
    vec3 *v = (vec3 *)luaL_checkudata(L, 1, "vec3");

    // some magic to call the correct method
    luaL_getmetatable(L, "vec3");
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
      if (!strcmp(n, "z")) {
        lua_pushnumber(L, v->z);
        return 1;
      }
    }

    return 1;
  }

  static int vec3__newindex(lua_State* L) {
    vec3 *v = (vec3 *)luaL_checkudata(L, 1, "vec3");

    const char *n = luaL_checkstring(L, 2);

    if (!strcmp(n, "x")) {
      v->x = lua_tonumber(L, 3);
    }
    if (!strcmp(n, "y")) {
      v->y = lua_tonumber(L, 3);
    }
    if (!strcmp(n, "z")) {
      v->z = lua_tonumber(L, 3);
    }
    
    return 0;
  }
}

void register_vec3lib(lua_State *L) {
  // constructor
  lua_register(L, "vec3", vec3_new);  

  luaL_newmetatable(L, "vec3");

  // push static methods
  //lua_pushcfunction(L, vec3__gc); lua_setfield(L, -2, "__gc");
  //lua_pushcfunction(L, vec3_reflect); lua_setfield(L, -2, "reflect");
  
  // push meta methods    
  lua_pushvalue(L, -1); lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, vec3__index      ); lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, vec3__add        ); lua_setfield(L, -2, "__add");
  lua_pushcfunction(L, vec3__unm        ); lua_setfield(L, -2, "__unm");
  lua_pushcfunction(L, vec3__sub        ); lua_setfield(L, -2, "__sub");
  lua_pushcfunction(L, vec3__mul        ); lua_setfield(L, -2, "__mul");
  lua_pushcfunction(L, vec3__newindex   ); lua_setfield(L, -2, "__newindex");

  // push instance methods        
  lua_pushcfunction(L, vec3_normalize   ); lua_setfield(L, -2, "normalize");
  lua_pushcfunction(L, vec3_dot         ); lua_setfield(L, -2, "dot");
  lua_pushcfunction(L, vec3_length      ); lua_setfield(L, -2, "length");
  lua_pushcfunction(L, vec3_reflect     ); lua_setfield(L, -2, "reflect");
  
  lua_pop(L, 1);
}
*/
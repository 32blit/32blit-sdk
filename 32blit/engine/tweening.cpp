#include <vector>
#include <algorithm>
/*
extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}
*/
#include "tweening.hpp"

namespace tge {
  /*
  std::vector<tge::Tween> tweens;
  uint32_t last_time;

  int lua_tween(lua_State* L) {
    int nargs = lua_gettop(L);

    Tween t;    
    t.variable  = lua_tostring(L, 1); // start value
    t.start     = lua_tonumber(L, 2); // start value
    t.end       = lua_tonumber(L, 3); // end value
    t.duration  = int32_t(lua_tonumber(L, 4) * 1000.0f); // duration
    lua_pop(L, nargs);

    tweens.push_back(t);

    return 0;
  }

  float tween_linear(uint32_t t, float b, float c, uint32_t d) {
    return c * t / d + b;
  }

  float tween_ease_in_quad(uint32_t t, float b, float c, uint32_t d) {
    float ft = float(t) / float(d);
    return -c * ft * (ft - 2) + b;
  }

  float tween_ease_out_quad(uint32_t t, float b, float c, uint32_t d) {
    float ft = float(t) / float(d);
    return c * ft * ft + b;
  }

  float tween_ease_in_out_quad(uint32_t t, float b, float c, uint32_t d) {
    float ft = (float)t / d / 2.0f;
    if (ft < 1) return c / 2 * ft * ft + b;
    ft--;
    return -c / 2 * (ft*(ft - 2) - 1) + b;
  }

  void update_tweens(lua_State* L, uint32_t time) {
    uint32_t elapsed_time = (uint32_t)(time - last_time);

    for (Tween &tween : tweens) {
      tween.elapsed += elapsed_time;

      float v = tween_linear(tween.elapsed, tween.start, tween.end, tween.duration);
      
      lua_pushinteger(L, (int32_t)v);
      lua_setglobal(L, tween.variable.c_str());
      
      
      if (tween.elapsed >= tween.duration) {
        // remove the tween
        tween.elapsed = 0;
      }
    }

    last_time = time;
  }*/

}
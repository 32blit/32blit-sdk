/*! \file timer.cpp
*/
#include "engine.hpp"
#include "timer.hpp"

namespace blit {

  std::vector<timer *> timers;

  timer::timer() {
    timers.push_back(this);
  }

  /**
   * Create a new timer.
   *
   * @param callback Callback function to trigger when timer has elapsed.
   * @param duration Duration of the timer in milliseconds.
   * @param loops Number of times the timer should repeat, -1 = forever.
   */
  timer::timer(timer_callback callback, uint32_t duration, uint16_t loops = -1) : callback(callback), duration(duration), loops(loops) {        
    started = blit::now();
    state = timer::RUNNING;
    timers.push_back(this);
  }

  /**
   * Start the timer.
   */
  void timer::start() { 
    this->state = RUNNING;  
  }

  /**
   * Stop the running timer.
   */
  void timer::stop() {
    this->state = STOPPED;
  }

  /**
   * Update all running timers, triggering any that have elapsed.
   *
   * @param time Time in milliseconds.
   */
  void update_timers(uint32_t time) {
    for (auto *t: timers) {      
      if (t->state == t->RUNNING && t->loops != 0) {
        if (time > (t->started + t->duration)) {              // timer triggered
          t->loops--;

          t->callback(*t);

          t->started = time;  // reset the start time correcting for any error

          //lua_rawgeti(L, LUA_REGISTRYINDEX, t->callback);
          //int result = lua_pcall(L, 0, 0, 0);

          ///if (result) {
            //panic(L);
          //}
        }
      }

      if (t->loops == 0) {
        t->state = t->FINISHED;
      }
    }
  }


  /*
  int lua_set_timer(lua_State* L) {
    int nargs = lua_gettop(L);

    uint8_t index = lua_tointeger(L, 1);
    Timer *t = &hws->timers[index];

    lua_pushvalue(L, 2);
    t->callback = luaL_ref(L, LUA_REGISTRYINDEX);
    t->duration = lua_tointeger(L, 3);    
    t->loops = nargs > 3 ? lua_tointeger(L, 4) : -1;
    t->started = now();
    t->state = t->RUNNING;
    lua_pop(L, nargs);    

    return 0;
  }

  */
}
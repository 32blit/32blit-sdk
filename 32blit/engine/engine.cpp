/*! \file engine.cpp
*/
#include "engine.hpp"
#include "timer.hpp"
#include "tweening.hpp"

namespace blit {

  void (*init)()                                    = nullptr;
  void (*update)(uint32_t time)                     = nullptr;
  void (*render)(uint32_t time)                     = nullptr;
  void (*set_screen_mode)(screen_mode new_mode)     = nullptr;
  uint32_t (*now)()                                 = nullptr;
  uint32_t (*random)()                              = nullptr;
  void (*debug)(std::string message)                = nullptr;
  int  (*debugf)(const char * psFormatString, ...) 	= nullptr;

  surface null_surface(nullptr, pixel_format::RGB565, size(0, 0));
  surface &fb = null_surface;

  uint32_t update_rate_ms = 10;
  uint32_t pending_update_time = 0;

  uint32_t render_rate_ms = 20;
  uint32_t pending_render_time = 0;

  uint32_t last_tick_time = 0;

  bool tick(uint32_t time) {
    bool has_rendered = false;

    if (last_tick_time == 0) {
      last_tick_time = time;
    }

    // update timers
    update_timers(time);
    update_tweens(time);

    // catch up on updates if any pending
    pending_update_time += (time - last_tick_time);
    while (pending_update_time >= update_rate_ms) {
      update(time - pending_update_time); // create fake timestamp that would have been accurate for the update event
      pending_update_time -= update_rate_ms;
    }

    // render if new frame due
    pending_render_time += (time - last_tick_time);
    if (pending_render_time >= render_rate_ms) {
      render(time);
      pending_render_time -= render_rate_ms;
      has_rendered = true;
    }

    last_tick_time = time;
    return has_rendered;
  }

}

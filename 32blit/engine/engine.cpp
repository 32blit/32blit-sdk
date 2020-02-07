/*! \file engine.cpp
*/
#include "engine.hpp"
#include "timer.hpp"
#include "tweening.hpp"

namespace blit {

  void (*init)()                                    = nullptr;
  void (*update)(uint32_t time)                     = nullptr;
  void (*render)(uint32_t time)                     = nullptr;
  void (*set_screen_mode)(ScreenMode new_mode)      = nullptr;
  uint32_t (*now)()                                 = nullptr;
  uint32_t (*random)()                              = nullptr;
  void (*debug)(std::string message)                = nullptr;
  int  (*debugf)(const char * psFormatString, ...) 	= nullptr;
  void (*switch_execution)()												= nullptr;

  int32_t (*open_file)(std::string file)          = nullptr;
  int32_t (*read_file)(uint32_t fh, uint32_t offset, uint32_t length, char* buffer) = nullptr;
  int32_t (*close_file)(uint32_t fh)              = nullptr;


  Surface null_surface(nullptr, PixelFormat::M, Size(0, 0));
  Surface &screen = null_surface;

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

    last_tick_time = time;

    return true;
  }

}

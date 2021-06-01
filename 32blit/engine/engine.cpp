/*! \file engine.cpp
*/
#include <cstdarg>

#include "engine.hpp"
#include "api_private.hpp"
#include "timer.hpp"
#include "tweening.hpp"

namespace blit {

  void (*init)()                                    = nullptr;
  void (*update)(uint32_t time)                     = nullptr;
  void (*render)(uint32_t time)                     = nullptr;

  void set_screen_mode(ScreenMode new_mode) {
    auto &new_screen = api.set_screen_mode(new_mode);
    screen = Surface(new_screen.data, new_screen.format, new_screen.bounds);
    screen.palette = new_screen.palette;
  }

  void set_screen_palette(const Pen *colours, int num_cols) {
    api.set_screen_palette(colours, num_cols);
  }

  uint32_t now() {
    return api.now();
  }

  uint32_t now_us() {
    return api.get_us_timer();
  }

  uint32_t us_diff(uint32_t from, uint32_t to) {
    if(to >= from)
      return to - from;
    else // wrap
      return (api.get_max_us_timer() - from) + to;
  }

  uint32_t random() {
    return api.random();
  }

  void debug(std::string message) {
    api.debug(message.c_str());
  }

  int debugf(const char * psFormatString, ...) {
    va_list args;
    va_start(args, psFormatString);

    // get length
    va_list tmp_args;
    va_copy(tmp_args, args);
    int len = vsnprintf(nullptr, 0, psFormatString, tmp_args) + 1;
    va_end(tmp_args);

    auto buf = new char[len];
    int ret = vsnprintf(buf, len, psFormatString, args);
    api.debug(buf);
    va_end(args);

    delete[] buf;
    return ret;
  }

  Surface null_surface(nullptr, PixelFormat::M, Size(0, 0));
  Surface &screen = null_surface;

  static const uint32_t update_rate_ms = 10;
  static uint32_t pending_update_time = 0;

  static uint32_t last_tick_time = 0;
  static uint32_t last_state = 0;

  extern std::vector<Timer *> timers;
  extern std::vector<Tween *> tweens;

  int tick(uint32_t time) {
    if (last_tick_time == 0) {
      last_tick_time = time;
    }

    bool was_menu_open = (last_state & Button::HOME) && !(api.buttons.state & Button::HOME);

    // skip time spent in the menu
    if (was_menu_open && time - last_tick_time > 10) {
      auto skipped_time = time - last_tick_time;
      last_tick_time = time;

      for (auto &timer : timers) {
        timer->started += skipped_time;
        timer->paused += skipped_time;
      }

      for (auto &tween : tweens) {
        tween->started += skipped_time;
        tween->paused += skipped_time;
      }
    }

    // update timers
    update_timers(time);
    update_tweens(time);

    // catch up on updates if any pending
    pending_update_time += (time - last_tick_time);
    while (pending_update_time >= update_rate_ms) {
      // button state changes
      uint32_t changed = api.buttons.state ^ last_state;

      api.buttons.pressed = changed & api.buttons.state;
      api.buttons.released = changed & last_state;
      last_state = api.buttons.state;

      update(time - pending_update_time); // create fake timestamp that would have been accurate for the update event
      pending_update_time -= update_rate_ms;
    }

    last_tick_time = time;

    return update_rate_ms - pending_update_time;
  }

  const char *get_launch_path() {
    return api.get_launch_path();
  }

}

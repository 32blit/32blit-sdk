#pragma once

#include <cstdint>
#include <string>

#include "../graphics/surface.hpp"

namespace blit {

  enum   ScreenMode  { lores, hires };
  extern Surface      &screen;


  extern void     (*init)             ();
  extern void     (*update)           (uint32_t time);
  extern void     (*render)           (uint32_t time);

  void     set_screen_mode(ScreenMode new_mode);
  uint32_t now();
  uint32_t random();

  void debug(std::string message);
  int debugf(const char * psFormatString, ...);

  bool tick(uint32_t time);
  void fast_tick(uint32_t time);

  // hal methods: read_file, reset

}

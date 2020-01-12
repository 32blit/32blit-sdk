#pragma once

#include <stdint.h>
#include <string>

#include "../graphics/surface.hpp"

namespace blit {

  enum   screen_mode  { lores, hires };
  extern surface      &fb;
  extern bool         halted;


  extern void     (*init)             ();
  extern void     (*update)           (uint32_t time);
  extern void     (*render)           (uint32_t time);
  extern void     (*set_screen_mode)  (screen_mode new_mode);
  extern uint32_t (*now)              ();
  extern uint32_t (*random)           ();
  extern void     (*debug)            (std::string message);
  extern int      (*debugf)           (const char * psFormatString, ...);
  extern uint32_t (*read_file)        (std::string file, uint32_t offset, uint32_t length, uint8_t* buffer);
  extern void     (*reset)            ();
  extern int32_t  (*open_file)        (std::string file);
  extern int32_t  (*read_file)        (uint32_t fh, uint32_t offset, uint32_t length, char* buffer);
  extern int32_t  (*close_file)       (uint32_t fh);  extern void     (*reset)            ();

  bool tick(uint32_t time);
  void fast_tick(uint32_t time);

  // hal methods: read_file, reset

}

#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32h7xx_hal.h"

#include "engine/engine.hpp"
#include "graphics/surface.hpp"

extern "C" {
  void LTDC_IRQHandler(void);
}

using namespace blit;

namespace display {

  extern screen_mode mode;
  extern bool needs_render;

  void init();  

  void enable_vblank_interrupt(); 

  void set_screen_mode(screen_mode new_mode);   
  void flip(const surface &source);

  void screen_init();
  void ltdc_init();

}


#endif
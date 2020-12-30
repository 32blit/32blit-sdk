  
#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32h7xx_hal.h"

#include "engine/engine.hpp"
#include "graphics/surface.hpp"

extern "C" {
  void LTDC_IRQHandler(void);
	void DMA2D_IRQHandler(void);
}

using namespace blit;

namespace display {

  extern ScreenMode mode;
  extern bool needs_render;

  void init();  

  void enable_vblank_interrupt(); 

  Surface &set_screen_mode(ScreenMode new_mode);
  void set_screen_palette(const Pen *colours, int num_cols);
  void flip(const Surface &source);

  void screen_init();
  void ltdc_init();
	
	bool is_dma2d_occupied(void);
	bool is_frameBuff_occupied(void);
	uint32_t get_dma2d_count(void);
	void set_dma2d_state(bool occupied);
	void set_frameBuff_state(bool occupied);
}


#endif

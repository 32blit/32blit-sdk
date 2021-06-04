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

namespace blit {
  struct SurfaceInfo;
}

namespace display {

  extern ScreenMode mode;
  extern bool needs_render;

  void init();

  void enable_vblank_interrupt();

  SurfaceInfo &set_screen_mode(ScreenMode new_mode);
  void set_screen_palette(const Pen *colours, int num_cols);
  void flip(const Surface &source);

  void screen_init();
  void ltdc_init();

  uint32_t get_dma2d_count(void);

  void dma2d_lores_flip_Step2(void);
  void dma2d_lores_flip_Step3(void);
  void dma2d_lores_flip_Step4(void);
}


#endif

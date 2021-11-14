#include <stdint.h>
#include <cstring>

#include "spi-st7272a.h"
#include "32blit.hpp"
#include "engine/api_private.hpp"

#include "display.hpp"
#include "stm32h7xx_ll_dma2d.h"

extern char __ltdc_start, __ltdc_end;
extern char __fb_start, __fb_end;


void LTDC_IRQHandler() {
  // check that interrupt triggered was line end event
  if(LTDC->ISR & LTDC_ISR_LIF)
  {
    // disable line interrupt and clear flag
    LTDC->IER &= ~LTDC_IT_LI;
    LTDC->ICR = LTDC_FLAG_LI;

    // flip the framebuffer to the ltdc buffer and request
    // a new frame to be rendered
    display::flip(blit::screen);
  }
}

void DMA2D_IRQHandler(void){
  //clear the flag

  DMA2D->IFCR = (uint32_t)(0x1F);
	uint32_t count = display::get_dma2d_count();
	switch(count){
		case 3:
			display::needs_render = true;
			display::dma2d_lores_flip_step2();
			break;
		case 2:
			display::dma2d_lores_flip_step3();
			break;
		case 1:
			display::dma2d_lores_flip_step4();
			break;
		case 0:   //highres, pal mode goto case 0 directly
			CLEAR_BIT(DMA2D->CR, DMA2D_CR_TCIE|DMA2D_CR_TEIE|DMA2D_CR_CEIE);//disable the DMA2D interrupt
			//set occupied to free
			if (display::mode != ScreenMode::lores){
        display::needs_render = true;
      }
	}

}

namespace display {
	void update_ltdc_for_mode();

  __IO uint32_t dma2d_step_count = 0;

  static Pen palette[256];
  // lo and hi res screen back buffers
  static const blit::SurfaceTemplate __fb_hires{(uint8_t *)&__fb_start, blit::Size(320, 240), blit::PixelFormat::RGB, nullptr};
  static const blit::SurfaceTemplate __fb_hires_pal{(uint8_t *)&__fb_start, blit::Size(320, 240), blit::PixelFormat::P, palette};
  static const blit::SurfaceTemplate __fb_lores{(uint8_t *)&__fb_start, blit::Size(160, 120), blit::PixelFormat::RGB, nullptr};

  static SurfaceInfo cur_surf_info; // used to pass screen info back through API

  ScreenMode mode = ScreenMode::lores;
  ScreenMode requested_mode = ScreenMode::lores;
  PixelFormat format = PixelFormat::RGB, requested_format = PixelFormat::RGB;

  bool needs_render = false;
  int palette_needs_update = 0;
  uint8_t palette_update_delay = 0;

  bool need_ltdc_mode_update = false;

  void init() {
    // TODO: replace interrupt setup with non HAL method
    HAL_NVIC_SetPriority(LTDC_IRQn, 4, 4);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);
		HAL_NVIC_SetPriority(DMA2D_IRQn,4,4 );//priority may be check again!
		HAL_NVIC_EnableIRQ(DMA2D_IRQn);

    ltdc_init();
    screen_init();

    needs_render = true;
  }

  void enable_vblank_interrupt() {
    // set new mode after rendering first frame in it
    if(mode != requested_mode || format != requested_format) {
      mode = requested_mode;
      format = requested_format;
      need_ltdc_mode_update = true;
    }

    // trigger interrupt when screen refresh reaches the 252nd scanline
    LTDC->LIPCR = 252;

    // enable line interrupt
    LTDC->IER |= LTDC_IT_LI;

    display::needs_render = false;
  }

  SurfaceInfo &set_screen_mode(ScreenMode new_mode) {
    requested_mode = new_mode;
    switch(new_mode) {
      case ScreenMode::lores:
        cur_surf_info = __fb_lores;
        break;
      case ScreenMode::hires:
        cur_surf_info = __fb_hires;
        break;
      case ScreenMode::hires_palette:
        cur_surf_info = __fb_hires_pal;
        break;
    }

    screen = Surface(cur_surf_info.data, cur_surf_info.format, cur_surf_info.bounds);
    screen.palette = cur_surf_info.palette;

    requested_format = screen.format;

    return cur_surf_info;
  }

  void set_screen_palette(const Pen *colours, int num_cols) {
    memcpy(palette, colours, num_cols * sizeof(blit::Pen));
    palette_update_delay = 1;
    palette_needs_update = num_cols;
  }

  bool set_screen_mode_format(ScreenMode new_mode, SurfaceTemplate &new_surf_template) {
    new_surf_template.data = (uint8_t *)&__fb_start;

    switch(new_mode) {
      case ScreenMode::lores:
        new_surf_template.bounds = __fb_lores.bounds;
        break;
      case ScreenMode::hires:
      case ScreenMode::hires_palette:
        new_surf_template.bounds = __fb_hires.bounds;
        break;
    }

    switch(new_surf_template.format) {
      case PixelFormat::RGB:
      case PixelFormat::RGB565:
        break;
      case PixelFormat::P:
        new_surf_template.palette = palette;
        palette_needs_update = 256;
        palette_update_delay = 0;
        break;

      default:
        return false;
    }

    requested_mode = new_mode;
    requested_format = new_surf_template.format;

    screen = Surface(new_surf_template.data, new_surf_template.format, new_surf_template.bounds);
    screen.palette = new_surf_template.palette;

    return true;
  }

  void dma2d_hires_flip(const Surface &source) {
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(source.data), 320 * 240 * 3);
    // set the transform type (clear bits 17..16 of control register)
    MODIFY_REG(DMA2D->CR, DMA2D_CR_MODE, LL_DMA2D_MODE_M2M_PFC);
    // set source pixel format (clear bits 3..0 of foreground format register)
    if(format == PixelFormat::RGB565)
      MODIFY_REG(DMA2D->FGPFCCR, DMA2D_FGPFCCR_CM, LL_DMA2D_INPUT_MODE_RGB565);
    else
      MODIFY_REG(DMA2D->FGPFCCR, DMA2D_FGPFCCR_CM, LL_DMA2D_INPUT_MODE_RGB888);
    // set source buffer address
    DMA2D->FGMAR = (uintptr_t)source.data;
    // set target pixel format (clear bits 3..0 of output format register)
    MODIFY_REG(DMA2D->OPFCCR, DMA2D_OPFCCR_CM, LL_DMA2D_OUTPUT_MODE_RGB565);
    // set target buffer address
    DMA2D->OMAR = (uintptr_t)&__ltdc_start;
    // set the number of pixels per line and number of lines
    DMA2D->NLR = (320 << 16) | (240);
    // set the source offset
    DMA2D->FGOR = 0;
    // set the output offset
    DMA2D->OOR = 0;
		//enable the DMA2D interrupt
	  SET_BIT(DMA2D->CR, DMA2D_CR_TCIE|DMA2D_CR_TEIE|DMA2D_CR_CEIE);
		//set DMA2d steps //set occupied
    dma2d_step_count = 0;
    // trigger start of dma2d transfer
    DMA2D->CR |= DMA2D_CR_START;
  }

  void dma2d_hires_pal_flip(const Surface &source) {
    // copy RGBA at quarter width
    // work as 32bit type to save some bandwidth
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(source.data), 320 * 240 * 1);
    // set the transform type (clear bits 17..16 of control register)
    MODIFY_REG(DMA2D->CR, DMA2D_CR_MODE, LL_DMA2D_MODE_M2M);
    // set source pixel format (clear bits 3..0 of foreground format register)
    MODIFY_REG(DMA2D->FGPFCCR, DMA2D_FGPFCCR_CM, LL_DMA2D_INPUT_MODE_ARGB8888);
    // set source buffer address
    DMA2D->FGMAR = (uintptr_t)source.data;
    // set target pixel format (clear bits 3..0 of output format register)
    MODIFY_REG(DMA2D->OPFCCR, DMA2D_OPFCCR_CM, LL_DMA2D_OUTPUT_MODE_ARGB8888);
    // set target buffer address
    DMA2D->OMAR = (uintptr_t)((uint32_t)&__ltdc_start + 320 * 240 * 1);
    // set the number of pixels per line and number of lines
    DMA2D->NLR = (80 << 16) | (240);
    // set the source offset
    DMA2D->FGOR = 0;
    // set the output offset
    DMA2D->OOR = 0;
    //enable the DMA2D interrupt
	  SET_BIT(DMA2D->CR, DMA2D_CR_TCIE|DMA2D_CR_TEIE|DMA2D_CR_CEIE);
		//set DMA2d steps //set occupied
    dma2d_step_count = 0;
    // trigger start of dma2d transfer
    DMA2D->CR |= DMA2D_CR_START;
    // update pal next, dma2d could work at same time
    if(palette_needs_update && palette_update_delay-- == 0) {
      for(int i = 0; i < palette_needs_update; i++) {
        LTDC_Layer1->CLUTWR = (i << 24) | (palette[i].b << 16) | (palette[i].g << 8) | palette[i].r;
      }
      LTDC->SRCR = LTDC_SRCR_IMR;
      palette_needs_update = 0;
    }
  }

  void dma2d_lores_flip(const Surface &source) {
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(source.data), 160 * 120 * 3);

    // palette update
    if(format == PixelFormat::P && palette_needs_update && palette_update_delay-- == 0) {
      SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(palette), palette_needs_update * 4);

      DMA2D->FGCMAR = (uint32_t)palette;
      MODIFY_REG(DMA2D->FGPFCCR, DMA2D_FGPFCCR_CCM | DMA2D_FGPFCCR_CS, (palette_needs_update - 1) << DMA2D_FGPFCCR_CS_Pos);

      DMA2D->FGPFCCR |= DMA2D_FGPFCCR_START;
      while(DMA2D->FGPFCCR & DMA2D_FGPFCCR_START);

      palette_needs_update = 0;
    }

    //Step 1.
    // set the transform type (clear bits 17..16 of control register)
    MODIFY_REG(DMA2D->CR, DMA2D_CR_MODE, LL_DMA2D_MODE_M2M_PFC);
    // set source pixel format (clear bits 3..0 of foreground format register)
    if(format == PixelFormat::RGB565)
      MODIFY_REG(DMA2D->FGPFCCR, DMA2D_FGPFCCR_CM, LL_DMA2D_INPUT_MODE_RGB565);
    else if(format == PixelFormat::P)
      MODIFY_REG(DMA2D->FGPFCCR, DMA2D_FGPFCCR_CM, LL_DMA2D_INPUT_MODE_L8);
    else
      MODIFY_REG(DMA2D->FGPFCCR, DMA2D_FGPFCCR_CM, LL_DMA2D_INPUT_MODE_RGB888);

    // set source buffer address
    DMA2D->FGMAR = (uintptr_t)source.data;
    // set target pixel format (clear bits 3..0 of output format register)
    MODIFY_REG(DMA2D->OPFCCR, DMA2D_OPFCCR_CM, LL_DMA2D_OUTPUT_MODE_RGB565);
    // set target buffer address
    DMA2D->OMAR = ((uintptr_t)&__ltdc_start)+320*120*2;
    // set the number of pixels per line and number of lines
    DMA2D->NLR = (1 << 16) | (160*120);
    // set the source offset
    DMA2D->FGOR = 0;
    // set the output offset
    DMA2D->OOR = 1;
	  SET_BIT(DMA2D->CR, DMA2D_CR_TCIE|DMA2D_CR_TEIE|DMA2D_CR_CEIE);//enable the DMA2D interrupt
		//set DMA2d steps //set occupied
    dma2d_step_count = 3;
    // trigger start of dma2d transfer
    DMA2D->CR |= DMA2D_CR_START;
  }

	void dma2d_lores_flip_step2(void){
		//Step 2.
			// set the transform type (clear bits 17..16 of control register)
		MODIFY_REG(DMA2D->CR, DMA2D_CR_MODE, LL_DMA2D_MODE_M2M);
			// set source pixel format (clear bits 3..0 of foreground format register)
		MODIFY_REG(DMA2D->FGPFCCR, DMA2D_FGPFCCR_CM, LL_DMA2D_INPUT_MODE_RGB565);
			// set source buffer address
		DMA2D->FGMAR = ((uintptr_t)&__ltdc_start)+320*120*2;
			// set target pixel format (clear bits 3..0 of output format register)
		MODIFY_REG(DMA2D->OPFCCR, DMA2D_OPFCCR_CM, LL_DMA2D_OUTPUT_MODE_RGB565);
			// set target buffer address
		DMA2D->OMAR =  ((uintptr_t)&__ltdc_start)+320*120*2 + 2;
			// set the number of pixels per line and number of lines
		DMA2D->NLR = (1 << 16) | (160*120);
			// set the source offset
		DMA2D->FGOR = 1;
			// set the output offset
		DMA2D->OOR = 1;
				// trigger start of dma2d transfer
		dma2d_step_count = 2;
		DMA2D->CR |= DMA2D_CR_START;
	}

	void dma2d_lores_flip_step3(void){
		//step 3.
		// set the transform type (clear bits 17..16 of control register)
    MODIFY_REG(DMA2D->CR, DMA2D_CR_MODE, LL_DMA2D_MODE_M2M);
    // set source pixel format (clear bits 3..0 of foreground format register)
    MODIFY_REG(DMA2D->FGPFCCR, DMA2D_FGPFCCR_CM, LL_DMA2D_INPUT_MODE_ARGB8888);
    // set source buffer address
    DMA2D->FGMAR = ((uintptr_t)&__ltdc_start)+320*120*2;
    // set target pixel format (clear bits 3..0 of output format register)
    MODIFY_REG(DMA2D->OPFCCR, DMA2D_OPFCCR_CM, LL_DMA2D_OUTPUT_MODE_ARGB8888);
    // set target buffer address
    DMA2D->OMAR =  ((uintptr_t)&__ltdc_start);
    // set the number of pixels per line and number of lines
    DMA2D->NLR = (160 << 16) | (120);
    // set the source offset
    DMA2D->FGOR = 0;
    // set the output offset
    DMA2D->OOR = 160;
		dma2d_step_count = 1;
			// trigger start of dma2d transfer
		DMA2D->CR |= DMA2D_CR_START;
	}

	void dma2d_lores_flip_step4(void){
		// set the transform type (clear bits 17..16 of control register)
    MODIFY_REG(DMA2D->CR, DMA2D_CR_MODE, LL_DMA2D_MODE_M2M);
    // set source pixel format (clear bits 3..0 of foreground format register)
    MODIFY_REG(DMA2D->FGPFCCR, DMA2D_FGPFCCR_CM, LL_DMA2D_INPUT_MODE_ARGB8888);//same as step 3, skip it
    // set source buffer address
    DMA2D->FGMAR = ((uintptr_t)&__ltdc_start);
    // set target pixel format (clear bits 3..0 of output format register)
    MODIFY_REG(DMA2D->OPFCCR, DMA2D_OPFCCR_CM, LL_DMA2D_OUTPUT_MODE_ARGB8888);//same as step 3, skip it
    // set target buffer address
    DMA2D->OMAR =  ((uintptr_t)&__ltdc_start)+320*2;
    // set the number of pixels per line and number of lines
    DMA2D->NLR = (160 << 16) | (120);
    // set the source offset
    DMA2D->FGOR = 160;
    // set the output offset
    DMA2D->OOR = 160;
		dma2d_step_count = 0;
		// trigger start of dma2d transfer
		DMA2D->CR |= DMA2D_CR_START;
	}

  void flip(const Surface &source) {
    // switch colour mode if needed
    if(need_ltdc_mode_update) {
      update_ltdc_for_mode();
      need_ltdc_mode_update = false;
    }

    if(mode == ScreenMode::lores) {
      dma2d_lores_flip(source);
    } else { // hires(_palette)
      if(format == PixelFormat::P)
        dma2d_hires_pal_flip(source);
      else
        dma2d_hires_flip(source);
    }
  }

  void screen_init() {
    ST7272A_RESET();
   // st7272a_set_bgr();
  }

  // configure ltdc peripheral setting up the clocks, pin states, panel
  // parameters, and layers
  void ltdc_init() {

    // enable ltdc clock
    __HAL_RCC_LTDC_CLK_ENABLE();

    const int h_sync = 4;
    const int h_back_porch = 43;
    const int h_active = 320;
    const int h_front_porch = 8;

    const int v_sync = 4;
    const int v_back_porch = 12;
    const int v_active = 240;
    const int v_front_porch = 2;

    // configure the panel timings and signal polarity
    LTDC->GCR &= ~LTDC_GCR_PCPOL;   // synch signal polarity setting
    // hsync and vsync
    LTDC->SSCR = ((h_sync                                           - 1) << 16) | (v_sync                                           - 1);
    // accumulated horizonal and vertical back porch
    LTDC->BPCR = ((h_sync + h_back_porch                            - 1) << 16) | (v_sync + v_back_porch                            - 1);
    // accumulated active width and height
    LTDC->AWCR = ((h_sync + h_back_porch + h_active                 - 1) << 16) | (v_sync + v_back_porch + v_active                 - 1);
    // accumulated total width and height
    LTDC->TWCR = ((h_sync + h_back_porch + h_active + h_front_porch - 1) << 16) | (v_sync + v_back_porch + v_active + v_front_porch - 1);

    // enable ltdc transfer and fifo underrun error interrupts
    LTDC->IER = LTDC_IT_TE | LTDC_IT_FU;

    // configure ltdc layer
    LTDC_Layer1->WHPCR &= ~(LTDC_LxWHPCR_WHSTPOS | LTDC_LxWHPCR_WHSPPOS);
    LTDC_Layer1->WHPCR = ((0 + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> 16U) + 1U) | ((320 + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> 16U)) << 16U));
    LTDC_Layer1->WVPCR &= ~(LTDC_LxWVPCR_WVSTPOS | LTDC_LxWVPCR_WVSPPOS);
    LTDC_Layer1->WVPCR  = ((0 + (LTDC->BPCR & LTDC_BPCR_AVBP) + 1U) | ((240 + (LTDC->BPCR & LTDC_BPCR_AVBP)) << 16U));
    LTDC_Layer1->PFCR   = LTDC_PIXEL_FORMAT_RGB565;
    LTDC_Layer1->DCCR   = 0xff000000;     // layer default color (back, 100% alpha)
    LTDC_Layer1->CFBAR  = (uint32_t)&__ltdc_start;  // frame buffer start address
    LTDC_Layer1->CFBLR  = ((320 * 2) << LTDC_LxCFBLR_CFBP_Pos) | (((320 * 2) + 7) << LTDC_LxCFBLR_CFBLL_Pos);  // frame buffer line length and pitch
    LTDC_Layer1->CFBLNR = 240;            // line count
    LTDC_Layer1->CACR   = 255;            // alpha
    LTDC_Layer1->CR    |= LTDC_LxCR_LEN;  // enable layer

    // reload shadow registers
    LTDC->SRCR = LTDC_SRCR_IMR;

    // enable LTDC
    LTDC->GCR |= LTDC_GCR_LTDCEN;
  }

  void update_ltdc_for_mode() {
    // hires palette is special
    if(mode != ScreenMode::lores && format == PixelFormat::P) {
      LTDC_Layer1->PFCR = LTDC_PIXEL_FORMAT_L8;
      LTDC_Layer1->CFBAR  = (uint32_t)&__ltdc_start + 320 * 240 * 1;  // frame buffer start address
      LTDC_Layer1->CFBLR  = ((320 * 1) << LTDC_LxCFBLR_CFBP_Pos) | (((320 * 1) + 7) << LTDC_LxCFBLR_CFBLL_Pos);  // frame buffer line length and pitch
      LTDC_Layer1->CR |= LTDC_LxCR_CLUTEN;
    } else {
      LTDC_Layer1->PFCR = LTDC_PIXEL_FORMAT_RGB565;
      LTDC_Layer1->CFBAR  = (uint32_t)&__ltdc_start;  // frame buffer start address
      LTDC_Layer1->CFBLR  = ((320 * 2) << LTDC_LxCFBLR_CFBP_Pos) | (((320 * 2) + 7) << LTDC_LxCFBLR_CFBLL_Pos);  // frame buffer line length and pitch
      LTDC_Layer1->CR &= ~LTDC_LxCR_CLUTEN;
    }

    LTDC->SRCR = LTDC_SRCR_IMR;
  }

	uint32_t get_dma2d_count(void){
		return dma2d_step_count;
	}
}

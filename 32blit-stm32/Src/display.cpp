#include <stdint.h>

#include "spi-st7272a.h"

#include "display.hpp"

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
    display::needs_render = true; 
  }
}

namespace display {  

  // lo and hi res screen back buffers
  Surface __fb_hires((uint8_t *)&__fb_start, PixelFormat::RGB, Size(320, 240));
  Surface __fb_lores((uint8_t *)&__fb_start, PixelFormat::RGB, Size(160, 120));

  ScreenMode mode = ScreenMode::lores;
  bool needs_render = false;

  void init() {
    // TODO: replace interrupt setup with non HAL method
    HAL_NVIC_SetPriority(LTDC_IRQn, 4, 4);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);

    ltdc_init();
    screen_init();

    enable_vblank_interrupt();
  }
  
  void enable_vblank_interrupt() {
    // trigger interrupt when screen refresh reaches the 252nd scanline
    LTDC->LIPCR = 252;

    // enable line interrupt
    LTDC->IER |= LTDC_IT_LI;

    display::needs_render = false;
  }

  void set_screen_mode(ScreenMode new_mode) {
    mode = new_mode;
    screen = mode == ScreenMode::hires ? __fb_hires : __fb_lores;
  }

  void flip(const Surface &source) {        
    // TODO: both flip implementations can we done via DMA2D which will save
    // a heap of CPU time.

    if(mode == ScreenMode::lores) {
      uint8_t *s = (uint8_t *)source.data;
      uint8_t *d = (uint8_t *)(&__ltdc_start);

      // pixel double the framebuffer to the ltdc buffer
      for(uint8_t y = 0; y < 120; y++) {
        // pixel double the current row horizontally
        for(uint8_t x = 0; x < 160; x++) {
          *d++ = *(s + 0);
          *d++ = *(s + 1);
          *d++ = *(s + 2);
          *d++ = *(s + 0);
          *d++ = *(s + 1);
          *d++ = *(s + 2);

          s += 3;
        }

        // copy the previous row to pixel double vertically
        uint32_t *dc = (uint32_t *)d;
        uint32_t c = 240;
        while(c--) {
          *dc = *(dc - 240);
          dc++;
        }
        
        d = (uint8_t *)dc;
      }
    }else{
      // copy the framebuffer data into the ltdc buffer, originally this
      // was done via memcpy but implementing it as a 32-bit copy loop
      // was much faster.
      uint32_t *s = (uint32_t *)source.data;
      uint32_t *d = (uint32_t *)(&__ltdc_start);
      uint32_t c = (320 * 240 * 3) >> 2;
      while(c--) {
        *d++ = *s++;
      }
    }

    // since the ltdc hardware pulls frame data directly over the memory bus
    // without passing through the mcu's cache layer we must invalidate the
    // affected area to ensure that all data has been committed into ram
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)&__ltdc_start, &__ltdc_end - &__ltdc_start);    
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

    // configure the panel timings and signal polarity
    LTDC->GCR &= ~LTDC_GCR_PCPOL;   // synch signal polarity setting
    LTDC->SSCR = (3 << 16) | 3;     // hsync and vsync
    LTDC->BPCR = (45 << 16) | 14;   // accumulated horizonal and vertical back porch
    LTDC->AWCR = (366 << 16) | 255; // accumulated active width and height
    LTDC->TWCR = (374 << 16) | 257; // accumulated total width and height

    // enable ltdc transfer and fifo underrun error interrupts
    LTDC->IER = LTDC_IT_TE | LTDC_IT_FU;

    // configure ltdc layer        
    LTDC_Layer1->WHPCR &= ~(LTDC_LxWHPCR_WHSTPOS | LTDC_LxWHPCR_WHSPPOS);
    LTDC_Layer1->WHPCR = ((1 + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> 16U) + 1U) | ((321 + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> 16U)) << 16U));
    LTDC_Layer1->WVPCR &= ~(LTDC_LxWVPCR_WVSTPOS | LTDC_LxWVPCR_WVSPPOS);
    LTDC_Layer1->WVPCR  = ((0 + (LTDC->BPCR & LTDC_BPCR_AVBP) + 1U) | ((241 + (LTDC->BPCR & LTDC_BPCR_AVBP)) << 16U));  
    LTDC_Layer1->PFCR   = LTDC_PIXEL_FORMAT_RGB888;  
    LTDC_Layer1->DCCR   = 0xff000000;     // layer default color (back, 100% alpha)
    LTDC_Layer1->CFBAR  = (uint32_t)&__ltdc_start;  // frame buffer start address
    LTDC_Layer1->CFBLR  = ((320 * 3) << LTDC_LxCFBLR_CFBP_Pos) | (((320 * 3) + 2) << LTDC_LxCFBLR_CFBLL_Pos);  // frame buffer line length and pitch
    LTDC_Layer1->CFBLNR = 240;            // line count
    LTDC_Layer1->CACR   = 255;            // alpha
    LTDC_Layer1->CR    |= LTDC_LxCR_LEN;  // enable layer

    // reload shadow registers
    LTDC->SRCR = LTDC_SRCR_IMR;     

    // enable LTDC      
    LTDC->GCR |= LTDC_GCR_LTDCEN;   
  }
}
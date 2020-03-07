#include <stdint.h>

#include "spi-st7272a.h"
#include "32blit.hpp"

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

  void update_ltdc_for_mode();

  // lo and hi res screen back buffers
  Surface __fb_hires((uint8_t *)&__fb_start, PixelFormat::RGB, Size(320, 240));
  Surface __fb_hires_pal((uint8_t *)&__fb_start, PixelFormat::P, Size(320, 240));
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

  Surface &set_screen_mode(ScreenMode new_mode) {
    mode = new_mode;
    switch(mode) {
      case ScreenMode::lores:
        screen = __fb_lores;
        break;
      case ScreenMode::hires:
        screen = __fb_hires;
        break;
      case ScreenMode::hires_palette:
        screen = __fb_hires_pal;
        break;
    }

    update_ltdc_for_mode();
    return screen;
  }

  void set_screen_palette(const Pen *colours, int num_cols) {
    if(mode != ScreenMode::hires_palette)
      return;

    for(int i = 0; i < num_cols; i++) {
      LTDC_Layer1->CLUTWR = (i << 24) | (colours[i].b << 16) | (colours[i].g << 8) | colours[i].r;
    }

    LTDC_Layer1->CR |= LTDC_LxCR_CLUTEN;
    LTDC->SRCR = LTDC_SRCR_IMR;
  }

  void dma2d_hires_flip(const Surface &source) {
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(source.data), 320 * 240 * 3); 

    // set the transform type (clear bits 17..16 of control register)
    DMA2D->CR &= 0xfcff;
    // set target pixel format (clear bits 3..0 of foreground format register)
    DMA2D->FGPFCCR &= 0xfff0;
    // set source buffer address
    DMA2D->FGMAR = (uintptr_t)source.data; 
    // set target pixel format (clear bits 3..0 of output format register)
    DMA2D->OPFCCR &= 0xfff0;
    // set target buffer address
    DMA2D->OMAR = (uintptr_t)&__ltdc_start;
    // set the number of pixels per line and number of lines    
    DMA2D->NLR = (320 << 16) | (240);
    // set the source offset
    DMA2D->FGOR = 0;
    // set the output offset
    DMA2D->OOR = 0;
    // trigger start of dma2d transfer
    DMA2D->CR |= DMA2D_CR_START;

    // wait for transfer to complete
    while(DMA2D->CR & DMA2D_CR_START) {      
      // never gets here!
    }
  }

  void dma2d_lores_flip(const Surface &source) {
    // this does not work... yet!
    /*SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(source.data), 320 * 240 * 3); 

    // set the transform type (clear bits 17..16 of control register)
    DMA2D->CR &= 0xfcff;
    // set target pixel format (clear bits 3..0 of foreground format register)
    DMA2D->FGPFCCR &= 0xfff0;
    // set source buffer address
    DMA2D->FGMAR = source.data; 
    // set target pixel format (clear bits 3..0 of output format register)
    DMA2D->OPFCCR &= 0xfff0;
    // set target buffer address
    DMA2D->OMAR = (&__ltdc_start) + (320 * 120 * 3) + 3; // halfway point
    // set the number of pixels per line and number of lines    
    DMA2D->NLR = (1 << 16) | (320 * 240 * 2);
    // set the source offset
    DMA2D->FGOR = 1;
    // set the output offset
    DMA2D->OOR = 1;
    // trigger start of dma2d transfer
    DMA2D->CR |= DMA2D_CR_START;

    // wait for transfer to complete
    while(DMA2D->CR & DMA2D_CR_START) {      
      // never gets here!
    }*/
  }

  void flip(const Surface &source) {        
    static uint32_t flip_time = 0;

    // TODO: both flip implementations can we done via DMA2D which will save
    // a heap of CPU time.

    uint32_t ltdc_buffer_size = 320 * 240 * 3;

    if(mode == ScreenMode::lores) {
      //dma2d_lores_flip(source);
      //screen.text(std::to_string(flip_time), minimal_font, Point(100,40));

      uint32_t flip_start = DWT->CYCCNT;

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
        uint32_t *cd = (uint32_t *)d;
        uint32_t *cs = (uint32_t *)(d - (320 * 3));
        while(cs < (uint32_t *)d) {
          *cd++ = *cs++;
        }        
        d += 320 * 3;
      }
      
      uint32_t flip_end = DWT->CYCCNT;
      flip_time = ((flip_end - flip_start) / 1000) * 1000;
    } else if(mode == ScreenMode::hires) {
            
      //screen.text(std::to_string(flip_time), minimal_font, Point(140,40));
      //uint32_t flip_start = DWT->CYCCNT;

      // perform flip with dma2d transfer
      dma2d_hires_flip(source);

      /*
        // alternative soft implementation
        // copy the framebuffer data into the ltdc buffer, originally this
        // was done via memcpy but implementing it as a 32-bit copy loop
        // was much faster.  
        uint32_t *s = (uint32_t *)source.data;
        uint32_t *d = (uint32_t *)(&__ltdc_start);
        uint32_t c = ltdc_buffer_size >> 2;
        while(c--) {
          *d++ = *s++;
        }
      */

      //uint32_t flip_end = DWT->CYCCNT;
      //flip_time = ((flip_end - flip_start) / 1000) * 1000;

      
    } else {
        // paletted
        uint32_t *s = (uint32_t *)source.data;
        uint32_t *d = (uint32_t *)(&__ltdc_start);
        uint32_t c = (320 * 240) >> 2;
        while(c--) {
          *d++ = *s++;
        }
    }

    // since the ltdc hardware pulls frame data directly over the memory bus
    // without passing through the mcu's cache layer we must invalidate the
    // affected area to ensure that all data has been committed into ram
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(&__ltdc_start), ltdc_buffer_size);    
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

  void update_ltdc_for_mode() {
    if(mode == ScreenMode::hires_palette) {
      LTDC_Layer1->PFCR = LTDC_PIXEL_FORMAT_L8;
      LTDC_Layer1->CR |= LTDC_LxCR_CLUTEN;
    } else {
      LTDC_Layer1->PFCR = LTDC_PIXEL_FORMAT_RGB888;
      LTDC_Layer1->CR &= ~LTDC_LxCR_CLUTEN;
    }

    LTDC->SRCR = LTDC_SRCR_IMR;
  }
}
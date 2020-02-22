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

  uint32_t flip_cycle_count;

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
    __disable_irq(); // Set PRIMASK
    uint8_t *s = (uint8_t *)source.data;
    uint32_t *d = (uint32_t *)(&__ltdc_start);

    if(mode == ScreenMode::lores) {
      // pixel double the framebuffer to the ltdc buffer
      for(uint8_t y = 0; y < 120; y++) {
        // pixel double the current row while converting from RGB to RGB565
        for(uint8_t x = 0; x < 160; x++) {
          uint16_t c = ((*s++ & 0b11111000) << 8) | ((*s++ & 0b11111100) << 3) | ((*s++ & 0b11111000) >> 3);        
          *(d) = c | (c << 16); *(d + 160) = c | (c << 16);
          d++;
        }
        d += 160; // skip the doubled row
      }
    }else{
      static uint8_t t = 0;
      t++;

      bool dither_spacial = false;

      uint32_t scc = DWT->CYCCNT;
      if(dither_spacial) {
        int8_t dither[4][4] = {
          { 0,  8,  2, 10},
          {12,  4, 14,  6},
          { 3, 11,  1,  9},
          {15,  7, 13,  5}
        };

        for(uint16_t y = 0; y < 240; y++) {
          uint8_t *pd = dither[y & 0b11];
          for(uint16_t x = 0; x < 320; x+=4) {
            int8_t dt = x;// + t;

            uint32_t dither = 0;// (pd[(dt + 1) & 0b11] << 24) | (pd[(dt + 0) & 0b11] << 16) | (pd[(dt + 3) & 0b11] << 8) | (pd[(dt + 2) & 0b11]);
            uint32_t red    = ((*(s + 3)) << 24) | ((*(s + 0)) << 16) | ((*(s +  9) << 8)) | (*(s + 6));
            uint32_t green  = ((*(s + 4)) << 24) | ((*(s + 1)) << 16) | ((*(s + 10) << 8)) | (*(s + 7));
            uint32_t blue   = ((*(s + 5)) << 24) | ((*(s + 2)) << 16) | ((*(s + 11) << 8)) | (*(s + 8));
            
            red   = __UQADD8(  red, dither);
            green = __UQADD8(green, dither);
            blue  = __UQADD8( blue, dither);

            uint32_t p12 = 
              ((red & 0xf8000000) >>  0) | ((green & 0xfc000000) >>  5) | ((blue & 0xf8000000) >> 11) | 
              ((red & 0x00f80000) >>  8) | ((green & 0x00fc0000) >> 13) | ((blue & 0x00f80000) >> 19);

            uint32_t p34 = 
              ((red & 0x0000f800) << 16) | ((green & 0x0000fc00) << 11) | ((blue & 0x0000f800) <<  5) | 
              ((red & 0x000000f8) <<  8) | ((green & 0x000000fc) <<  3) | ((blue & 0x000000f8) >>  3);

            *d = p12;
            d++;
            *d = p34;
            d++;

            s += 12;
          }
        }
      } else {
        uint32_t c = (320 * 240) >> 1;
        while(c--) {
          uint16_t c1 = ((*s++ & 0b11111000) << 8) | ((*s++ & 0b11111100) << 3) | ((*s++ & 0b11111000) >> 3);
          uint16_t c2 = ((*s++ & 0b11111000) << 8) | ((*s++ & 0b11111100) << 3) | ((*s++ & 0b11111000) >> 3);
          *d++ = c2 << 16 | c1;
        }
      }
      flip_cycle_count = DWT->CYCCNT - scc;
    }

    __enable_irq(); // Clear PRIMASK
    // since the ltdc hardware pulls frame data directly over the memory bus
    // without passing through the mcu's cache layer we must invalidate the
    // affected area to ensure that all data has been committed into ram
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(&__ltdc_start), 320 * 240 * 2);    
  }

  void screen_init() {
    ST7272A_RESET();
    st7272a_set_bgr();
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
    LTDC_Layer1->PFCR   = LTDC_PIXEL_FORMAT_RGB565;  
    LTDC_Layer1->DCCR   = 0xff000000;     // layer default color (back, 100% alpha)
    LTDC_Layer1->CFBAR  = (uint32_t)&__ltdc_start;  // frame buffer start address
    LTDC_Layer1->CFBLR  = ((320 * 2) << LTDC_LxCFBLR_CFBP_Pos) | (((320 * 2) + 2) << LTDC_LxCFBLR_CFBLL_Pos);  // frame buffer line length and pitch
    LTDC_Layer1->CFBLNR = 240;            // line count
    LTDC_Layer1->CACR   = 255;            // alpha
    LTDC_Layer1->CR    |= LTDC_LxCR_LEN;  // enable layer

    // reload shadow registers
    LTDC->SRCR = LTDC_SRCR_IMR;     

    // enable LTDC      
    LTDC->GCR |= LTDC_GCR_LTDCEN;   
  }
}
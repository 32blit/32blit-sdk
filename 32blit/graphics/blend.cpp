#include <cstdint>
#include <cstring>

#include "surface.hpp"

#ifdef WIN32 
#define __attribute__(A)
#endif

// note:
// for performance reasons none of the blending functions make any attempt 
// to validate input, adhere to clipping, or source/destination bounds. it 
// is assumed that all validation has been done by the caller. 

namespace blit {

  __attribute__((always_inline)) inline uint32_t alpha(const uint32_t &a1, const uint32_t &a2) {
    return ((a1 + 1) * (a2 + 1)) >> 8;
  }

  __attribute__((always_inline)) inline uint32_t alpha(const uint32_t &a1, const uint32_t &a2, const uint32_t &a3) {
    return ((a1 + 1) * (a2 + 1) * (a3 + 1)) >> 16;
  }

  __attribute__((always_inline)) inline uint8_t blend(const uint8_t &s, const uint8_t &d, const uint8_t &a) {
    return d + ((a * (s - d) + 127) >> 8);    
  }

  __attribute__((always_inline)) inline void blend_rgba_rgb(const Pen *s, uint8_t *d, const uint8_t &a, uint32_t c) {      
    if (c == 1) { 
      // fast case for single pixel draw
      *d = blend(s->r, *d, a); d++;
      *d = blend(s->g, *d, a); d++;
      *d = blend(s->b, *d, a); d++;
      return;
    }

    if (c <= 4) {
      // fast case for small number of pixels
      while (c--) {
        *d = blend(s->r, *d, a); d++;
        *d = blend(s->g, *d, a); d++;
        *d = blend(s->b, *d, a); d++;
      }
      return;
    }

    // create packed 32bit source
    // s32 now contains RGBA
    uint32_t s32 = *((uint32_t*)(s));
    // replace A with R so s32 is now RGBR
    s32 = (s32 & 0x00ffffff) | ((s32 & 0x000000ff) << 24);

    // if destination is not double-word aligned copy at most three bytes until it is
    uint8_t* de = d + c * 3;
    while (uintptr_t(d) & 0b11) {
      *d = blend((s32 & 0xff), *d, a); d++;
      // rotate the aligned rgbr/gbrg/brgb quad
      s32 >>= 8; s32 |= uint8_t(s32 & 0xff) << 24;
    }
        
    // destination is now double-word aligned
    if (d < de) {      
      // get a double-word aligned pointer to the destination surface
      uint32_t *d32 = (uint32_t*)d;

      // copy four bytes at a time until we have fewer than four bytes remaining
      uint32_t c32 = uint32_t(de - d) >> 2;
      while (c32--) {
        uint32_t dd32 = *d32;

        *d32++ = blend((s32 & 0xff), (dd32 & 0xff), a) | 
                (blend((s32 & 0xff00) >> 8, (dd32 & 0xff00) >> 8, a) << 8) | 
                (blend((s32 & 0xff0000) >> 16, (dd32 & 0xff0000) >> 16, a) << 16) |
                (blend((s32 & 0xff000000) >> 24, (dd32 & 0xff000000) >> 24, a) << 24);

        // rotate the aligned rgbr/gbrg/brgb quad        
        s32 >>= 8; s32 |= uint8_t(s32 & 0xff) << 24;
      }

      // copy the trailing bytes as needed
      d = (uint8_t*)de;
      while (d < de) {
        *d = blend((s32 & 0xff), *d, a); s32 >>= 8; d++;
      }
    }
  }

  __attribute__((always_inline)) inline void copy_rgba_rgb(const Pen* s, uint8_t *d, uint32_t c) {
    if (c == 1) { 
      // fast case for single pixel draw
      *(d + 0) = s->r; *(d + 1) = s->g; *(d + 2) = s->b; 
      return;
    }
    
    if (c <= 4) {
      // fast case for small number of pixels
      do {
        *(d + 0) = s->r; *(d + 1) = s->g; *(d + 2) = s->b; d += 3;
      } while (--c);      
      return;
    }    

    // create packed 32bit source
    // s32 now contains RGBA
    uint32_t s32 = *((uint32_t*)(s));
    // replace A with R so s32 is now RGBR
    s32 = (s32 & 0x00ffffff) | ((s32 & 0x000000ff) << 24);

    // if destination is not double-word aligned copy at most three bytes until it is
    uint8_t* de = d + c * 3;
    while (uintptr_t(d) & 0b11) {
      *d = s32 & 0xff000000; d++;
      // rotate the aligned rgbr/gbrg/brgb quad
      s32 >>= 8; s32 |= uint8_t(s32 & 0xff) << 24;
    }
        
    // destination is now double-word aligned
    if (d < de) {      
      // get a double-word aligned pointer to the destination surface
      uint32_t *d32 = (uint32_t*)d;

      // copy four bytes at a time until we have fewer than four bytes remaining
      uint32_t c32 = uint32_t(de - d) >> 2;
      while (c32--) {
        *d32++ = s32;
        // rotate the aligned rgbr/gbrg/brgb quad        
        s32 >>= 8; s32 |= uint8_t(s32 & 0xff) << 24;
      }

      // copy the trailing bytes as needed
      d = (uint8_t*)de;
      while (d < de) {
        *d = (s32 & 0xff); s32 >>= 8; d++;
      }
    }
  }

  void RGBA_RGBA(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    uint8_t* d = dest->data + (off * 4);
    uint8_t* m = dest->mask ? dest->mask->data + off : nullptr;    

    uint16_t a1 = alpha(pen->a, dest->alpha);
    do {
      uint16_t a = m ? alpha(a1, *m++) : a1;

      if (a >= 255) {
        *d++ = pen->r; *d++ = pen->g; *d++ = pen->b; *d++ = 255;
      } else if (a > 0) {
        *d = blend(pen->r, *d, a); d++;
        *d = blend(pen->g, *d, a); d++;
        *d = blend(pen->b, *d, a); d++;
        *d = blend(pen->a, *d, a); d++;
      }else{
        d += 4;
      }
    } while (--cnt);
  }

  void RGBA_RGB(const Pen* pen, const Surface* dest, uint32_t off, uint32_t c) {
    uint8_t* d = dest->data + (off * 3);
    uint8_t* m = dest->mask ? dest->mask->data + off : nullptr;    
  
    uint16_t a = alpha(pen->a, dest->alpha);
    if (!m) {
      // no mask
      if (a >= 255) {
        // no alpha, just copy
        copy_rgba_rgb(pen, d, c);
      }
      else {
        // alpha, blend
        blend_rgba_rgb(pen, d, a, c);
      }
    } else {
      // mask enabled, slow blend
      do {
        uint16_t ma = alpha(a, *m++);
        blend_rgba_rgb(pen, d, ma, 1);
        d += 3;
      } while (--c);
    }
  }

  void P_P(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    uint8_t* d = dest->data + off;

    do {
      if (pen->a != 0) {
        *d = pen->a;
      }
      d++;
    } while (--cnt);
  }

  void M_M(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    uint8_t* d = dest->data + off;

    do {
      *d = blend(pen->a, *d, dest->alpha); d++;
    } while (--cnt);
  }


  void RGBA_RGBA(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt, int32_t src_step) {
    uint8_t* s = src->palette ? src->data + soff : src->data + (soff * 4);
    uint8_t* d = dest->data + (doff * 3);
    uint8_t* m = dest->mask ? dest->mask->data + doff : nullptr;    

    do {
      Pen *pen = src->palette ? &src->palette[*s] : (Pen *)s;

      uint16_t a = m ? alpha(pen->a, *m++, dest->alpha) : alpha(pen->a, dest->alpha);

      if (a >= 255) {
        *d++ = pen->r; *d++ = pen->g; *d++ = pen->b; d++;
      } else if (a > 0) {
        *d = blend(pen->r, *d, a); d++;
        *d = blend(pen->g, *d, a); d++;
        *d = blend(pen->b, *d, a); d++;
        *d = blend(pen->b, *d, a); d++;
      }else{
        d += 4;
      }       

      s += src->palette ? 1 : 4;
    } while (--cnt);
  }

  void RGBA_RGB(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt, int32_t src_step) {
    uint8_t* s = src->palette ? src->data + soff : src->data + (soff * 4);
    uint8_t* d = dest->data + (doff * 3);
    uint8_t* m = dest->mask ? dest->mask->data + doff : nullptr;    

    do {
      Pen *pen = src->palette ? &src->palette[*s] : (Pen *)s;

      uint16_t a = m ? alpha(pen->a, *m++, dest->alpha) : alpha(pen->a, dest->alpha);

      if (a >= 255) {
        *d++ = pen->r; *d++ = pen->g; *d++ = pen->b;
      } else if (a > 0) {
        *d = blend(pen->r, *d, a); d++;
        *d = blend(pen->g, *d, a); d++;
        *d = blend(pen->b, *d, a); d++;
      }else{
        d += 3;
      }       

      s += src->palette ? 1 : 4;
    } while (--cnt);
  }

  void P_P(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt, int32_t src_step) {
    uint8_t *s = src->data + soff;
    uint8_t *d = dest->data + doff;

    do {
      if (*s != 0) {
        *d = *s;
      }
      d++; s += src_step;
    } while (--cnt);
  }

  void M_M(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt, int32_t src_step) {
    uint8_t *s = src->data + soff;
    uint8_t *d = dest->data + doff;

    do {
      *d = blend(*s, *d, dest->alpha); d++;
      s += src_step;
    } while (--cnt);
  }
}
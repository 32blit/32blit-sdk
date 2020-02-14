#include <stdint.h>
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

  void RGBA_RGBA(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    uint8_t* d = dest->data + (off * 4);
    uint8_t* m = dest->mask ? dest->mask->data + off : nullptr;    

    do {
      uint16_t a = m ? alpha(pen->a, *m++, dest->alpha) : alpha(pen->a, dest->alpha);

      if (a >= 255) {
        *d++ = pen->r; *d++ = pen->g; *d++ = pen->b; d++;
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

  void RGBA_RGB(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    uint8_t* d = dest->data + (off * 3);
    uint8_t* m = dest->mask ? dest->mask->data + off : nullptr;    
  
    do {
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
    } while (--cnt);
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
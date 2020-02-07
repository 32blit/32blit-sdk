#include <stdint.h>
#include <cstring>

#include "surface.hpp"

#ifdef WIN32 
#define __attribute__(A)
#endif

// note: for performance reasons none of the blending functions make any attempt 
// to validate input, adhere to clipping, or source/destination bounds. It 
// is assumed that all validation has been done by the caller. 

namespace blit {
  
  void RGBA_RGBA(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    uint8_t* d = dest->data + off + off + off + off;
    uint8_t* m = dest->mask ? dest->mask->data + off : nullptr;

    do {
      uint8_t a = (pen->a * (*(d + 3))) >> 8;
      if (m) { a = (a * *m++) >> 8; }
      if (dest->alpha != 255) { a = (a * dest->alpha) >> 8; }

      if (a == 255) {
        *d++ = pen->r; *d++ = pen->g; *d++ = pen->b; d++;
      } else if (a == 0) {
        d += 4;
      } else {
        uint8_t ia = 255 - a;
        *d++ = ((pen->r * a) + (*d * ia)) >> 8;
        *d++ = ((pen->g * a) + (*d * ia)) >> 8;
        *d++ = ((pen->b * a) + (*d * ia)) >> 8;
        *d++ = 255 - (((255 - pen->a) * (255 - *d)) >> 8);
      }      
    } while (--cnt);
  }

  void RGBA_RGB(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    uint8_t* d = dest->data + off + off + off;
    uint8_t* m = dest->mask ? dest->mask->data + off : nullptr;

    do {
      uint8_t a = pen->a;
      if (m) { a = (a * *m++) >> 8; }
      if (dest->alpha != 255) { a = (a * dest->alpha) >> 8; }

      if (a == 255) {
        *d++ = pen->r; *d++ = pen->g; *d++ = pen->b;
      } else if (a == 0) {
        d += 3;        
      } else {
        uint8_t ia = 255 - a;
        *d++ = ((pen->r * a) + (*d * ia)) >> 8;
        *d++ = ((pen->g * a) + (*d * ia)) >> 8;
        *d++ = ((pen->b * a) + (*d * ia)) >> 8;
      }
    } while (--cnt);
  }

  void P_P(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    uint8_t* d = dest->data + off;

    do {
      if (pen->a != 0) {
        *d++ = pen->a;
      }
      
    } while (--cnt);
  }

  void M_M(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    uint8_t* d = dest->data + off;

    do {
      *d++ = pen->a;
    } while (--cnt);
  }


  void RGBA_RGBA(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt) {
    uint8_t* s = src->data + soff + soff + soff + soff;
    uint8_t* d = dest->data + doff + doff + doff;
    uint8_t* m = dest->mask ? dest->mask->data + doff : nullptr;

    do {
      uint8_t a = ((*(s + 3)) * (*(d + 3))) >> 8;
      if (m) { a = (a * *m++) >> 8; }
      if (dest->alpha != 255) { a = (a * dest->alpha) >> 8; }

      if (a == 255) {
        *d++ = *s++; *d++ = *s++; *d++ = *s++; s++; d++;
      } else if (a == 0) {
        d += 4;
      } else {
        uint8_t ia = 255 - a;
        *d++ = ((*s++ * a) + (*d * ia)) >> 8;
        *d++ = ((*s++ * a) + (*d * ia)) >> 8;
        *d++ = ((*s++ * a) + (*d * ia)) >> 8;
        *d++ = 255 - (((255 - *s++) * (255 - *d)) >> 8);
      }
    } while (--cnt);
  }

  void RGBA_RGB(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt) {
    uint8_t* s = src->data + soff + soff + soff + soff;
    uint8_t* d = dest->data + doff + doff + doff;
    uint8_t* m = dest->mask ? dest->mask->data + doff : nullptr;

    do {
      uint8_t a = (*(s + 3));
      if (m) { a = (a * *m++) >> 8; }
      if (dest->alpha != 255) { a = (a * dest->alpha) >> 8; }

      if (a == 255) {
        *d++ = *s++; *d++ = *s++; *d++ = *s++; s++;
      } else if (a == 0) {
        d += 3;
      } else {
        uint8_t ia = 255 - a;
        *d++ = ((*s++ * a) + (*d * ia)) >> 8;
        *d++ = ((*s++ * a) + (*d * ia)) >> 8;
        *d++ = ((*s++ * a) + (*d * ia)) >> 8;
        s++;
      }
    } while (--cnt);
  }

  void P_P(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt) {
    uint8_t *s = src->data + soff;
    uint8_t *d = dest->data + doff;

    do {
      if (*s != 0) {
        *d++ = *s++;
      }

    } while (--cnt);
  }

  void M_M(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt) {
    uint8_t *s = src->data + soff;
    uint8_t *d = dest->data + doff;

    do {
      *d++ = *s++;
    } while (--cnt);
  }
}
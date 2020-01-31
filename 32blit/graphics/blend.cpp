#include <stdint.h>
#include <cstring>

#include "surface.hpp"
#include "../types/pixel_format.hpp"

// IMPORTANT NOTE
//
// For performance reasons none of the blending functions make any attempt 
// to validate input, adhere to clipping, or source/destination bounds. It 
// is assumed that all validation has been done by the caller. 

namespace blit {

  #define RGB_BLEND(sr, sg, sb, dr, dg, db, a) \
    uint8_t r = (sr * a) >> 8; \
    uint8_t g = (sg * a) >> 8; \
    uint8_t b = (sb * a) >> 8; \
    uint16_t ia = 256 - a; \
    dr = (r + ((dr * ia) >> 8)); \
    dg = (g + ((dg * ia) >> 8)); \
    db = (b + ((db * ia) >> 8)); \

  #define COMBINE_ALPHA(ca, sa, ga, pm) \
    uint16_t ca = ((sa) * ga) >> 8; \
    if (pm) { \
      ca = (ca * (*pm + 1)) >> 8; \
      pm++; \
    }
    
  #define COMBINE_ALPHA_MASK(ca, sa, ga, pm) \
    uint16_t ca = (sa * ga) >> 8; \
    ca = (ca * (*pm++ + 1)) >> 8;
  
  #define COMBINE_ALPHA_NO_MASK(ca, sa, ga) \
    uint16_t ca = (sa * ga) >> 8;

  //
  // single colour blend functions
  //
  void RGBA_RGB(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count) {
    rgba      *ps = (rgba *)pen;              // source pointer
    rgb       *pd = (rgb *)dest->data;        // destination pointer
    uint8_t   *pm = nullptr;                  // mask pointer
    uint16_t   ga = dest->alpha + 1;          // global alpha

    if (dest->mask) {
      pm = dest->mask->data;
      pm += offset;
    }

    pd += offset;

    do {
      COMBINE_ALPHA(a, ps->a, ga, pm)

      if (a == 255) {
        pd->r = ps->r; pd->g = ps->g; pd->b = ps->b;
      }else if(a == 0) {
      }else {
        RGB_BLEND(ps->r, ps->g, ps->b, pd->r, pd->g, pd->b, a)
      }

      pd++;
    } while (--count);
  }

  void RGBA_RGBA(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count) {
    rgba      *ps = (rgba *)pen;              // source pointer
    rgba      *pd = (rgba *)dest->data;       // destination pointer
    uint8_t   *pm = nullptr;                  // mask pointer
    uint16_t   ga = dest->alpha + 1;          // global alpha

    if (dest->mask) {
      pm = dest->mask->data;
      pm += offset;
    }

    pd += offset;

    do {
      COMBINE_ALPHA(a, ps->a, ga, pm)

      if (a == 255) {
        pd->r = ps->r; pd->g = ps->g; pd->b = ps->b; pd->a = 255;
      } else if (a == 0) {
      } else {
        RGB_BLEND(ps->r, ps->g, ps->b, pd->r, pd->g, pd->b, a)
      }

      pd++;
    } while (--count);
  }

  void RGBA_RGB565(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count) {
    rgba      *ps = (rgba *)pen;              // source pointer
    uint16_t  *pd = (uint16_t *)dest->data;   // destination pointer
    uint8_t   *pm = nullptr;                  // mask pointer
    uint16_t   ga = dest->alpha + 1;          // global alpha

    pd += offset;

    if (dest->mask) {                         // mask
      pm = dest->mask->data;
      pm += offset;

      do {
        COMBINE_ALPHA_MASK(a, ps->a, ga, pm)

        if (a == 255) {
          *pd = ((ps->r & 0b11111000) << 8) | ((ps->g & 0b11111000) << 3) | (ps->b >> 3);
        } else if (a == 0) {
        } else {
          uint8_t dr = ((*pd) >> 8) & 0b11111000;
          uint8_t dg = ((*pd) >> 3) & 0b11111000;
          uint8_t db = ((*pd) << 3) & 0b11111000;

          RGB_BLEND(ps->r, ps->g, ps->b, dr, dg, db, a)

          *pd = ((dr >> 3) << 11) | ((dg >> 2) << 5) | (db >> 3);
        }

        pd++;
      } while (--count);
    } else {                                  // no mask
      COMBINE_ALPHA_NO_MASK(a, ps->a, ga)

      if (a == 0)
        return;

      if (a == 255) {
        uint16_t pen565 = ((ps->r & 0b11111000) << 8) | ((ps->g & 0b11111000) << 3) | (ps->b >> 3);
        do {
          *pd++ = pen565;
        } while (--count);
      }else{
        do {
          uint8_t dr = ((*pd) >> 8) & 0b11111000;
          uint8_t dg = ((*pd) >> 3) & 0b11111000;
          uint8_t db = ((*pd) << 3) & 0b11111000;

          RGB_BLEND(ps->r, ps->g, ps->b, dr, dg, db, a)

          *pd++ = ((dr >> 3) << 11) | ((dg >> 2) << 5) | (db >> 3);
        } while (--count);
      }
    }
  }

  void RGBA_M(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count) {
    uint8_t v = *pen;
    uint8_t *pm = (uint8_t *)dest->data;

    pm += offset;

    uint16_t a = dest->alpha + 1;
    v = (v * a) >> 8;

    uint16_t ia = 256 - a;

    do {
      *pm = v + ((*pm * ia) >> 8);
      pm++;
    } while (--count);
  }

  void P_P(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count) {
    uint8_t  v = *pen;
    uint8_t *pd = (uint8_t *)dest->data;        // destination pointer

    std::memset(pd, v, count);
  }


  //
  // blit blend functions
  //
  void RGBA_RGB(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step) {
    rgba      *ps = (rgba *)src->data;        // source pointer
    rgb       *pd = (rgb *)dest->data;        // destination pointer
    uint8_t   *pm = nullptr;                  // mask pointer
    uint16_t   ga = dest->alpha + 1;          // global alpha

    ps += src_offset;
    pd += dest_offset;

    if (dest->mask) {
      pm = dest->mask->data;
      pm += dest_offset;

      do {
        COMBINE_ALPHA(a, ps->a, ga, pm)

        if (a == 255) {
          pd->r = ps->r; pd->g = ps->g; pd->b = ps->b;
        } else if (a == 0) {
        } else {
          RGB_BLEND(ps->r, ps->g, ps->b, pd->r, pd->g, pd->b, a)
        }

        ps += src_step;
        pd++;
      } while (--count);      
    } else {
      do {
        COMBINE_ALPHA_NO_MASK(a, ps->a, ga)
          
        if (a == 255) {
          pd->r = ps->r; pd->g = ps->g; pd->b = ps->b;
        } else if (a == 0) {
        } else {
          RGB_BLEND(ps->r, ps->g, ps->b, pd->r, pd->g, pd->b, a)
        }

        ps += src_step;
        pd++;
      } while (--count);
    }
  }  

  void RGBA_RGB565(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step) {
    rgba      *ps = (rgba *)src->data;        // source pointer
    uint16_t  *pd = (uint16_t *)dest->data;   // destination pointer
    uint8_t   *pm = nullptr;                  // mask pointer
    uint16_t   ga = dest->alpha + 1;          // global alpha

    if (dest->mask) {
      pm = dest->mask->data;
      pm += dest_offset;
    }

    ps += src_offset;
    pd += dest_offset;

    do {
      COMBINE_ALPHA(a, ps->a, ga, pm)

      if (a == 255) {
        *pd = ((ps->r & 0b11111000) << 8) | ((ps->g & 0b11111100) << 3) | (ps->b >> 3);
      } else if (a == 0) {
      } else {
        uint8_t dr = ((*pd) >> 8) & 0b11111000;
        uint8_t dg = ((*pd) >> 3) & 0b11111100;
        uint8_t db = ((*pd) << 3) & 0b11111000;

        RGB_BLEND(ps->r, ps->g, ps->b, dr, dg, db, a)

        *pd = ((dr >> 3) << 11) | ((dg >> 2) << 5) | (db >> 3);
      }

      ps += src_step;
      pd++;
    } while (--count);
  }

 
  void P_RGBA(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step) {
    uint8_t   *ps = (uint8_t *)src->data;     // source pointer
    rgba      *pd = (rgba *)dest->data;        // destination pointer
    uint8_t   *pm = nullptr;                  // mask pointer
    uint16_t   ga = dest->alpha + 1;          // global alpha

    if (dest->mask) {
      pm = dest->mask->data;
      pm += dest_offset;
    }

    ps += src_offset;
    pd += dest_offset;

    rgba* palette = src->palette.data();

    do {
      rgba *pps = &palette[*ps];

      COMBINE_ALPHA(a, pps->a, ga, pm)

      if (a == 255) {
        pd->r = pps->r; pd->g = pps->g; pd->b = pps->b;
      } else if (a == 0) {
      } else {
        RGB_BLEND(pps->r, pps->g, pps->b, pd->r, pd->g, pd->b, a)
      }

      ps += src_step;
      pd++;
    } while (--count);
  }
  
  void P_RGB(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step) {
    uint8_t   *ps = (uint8_t *)src->data;     // source pointer
    rgb       *pd = (rgb *)dest->data;        // destination pointer
    uint8_t   *pm = nullptr;                  // mask pointer
    uint16_t   ga = dest->alpha + 1;          // global alpha

    if (dest->mask) {
      pm = dest->mask->data;
      pm += dest_offset;
    }

    ps += src_offset;
    pd += dest_offset;

    rgba* palette = src->palette.data();

    do {
      rgba *pps = &palette[*ps];

      COMBINE_ALPHA(a, pps->a, ga, pm)

      if (a == 255) {
        pd->r = pps->r; pd->g = pps->g; pd->b = pps->b;
      } else if (a == 0) {
      } else {
        RGB_BLEND(pps->r, pps->g, pps->b, pd->r, pd->g, pd->b, a)
      }

      ps += src_step;
      pd++;
    } while (--count);
  }

  void P_RGB565(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step) {
    uint8_t   *ps = (uint8_t *)src->data;     // source pointer
    uint16_t  *pd = (uint16_t *)dest->data;   // destination pointer
    uint8_t   *pm = nullptr;                  // mask pointer
    uint16_t   ga = dest->alpha + 1;          // global alpha

    if (dest->mask) {
      pm = dest->mask->data;
      pm += dest_offset;
    }

    ps += src_offset;
    pd += dest_offset;

    rgba* palette = src->palette.data();

    do {
      rgba *pps = &palette[*ps];

      COMBINE_ALPHA(a, pps->a, ga, pm)

      if (a == 255) {
        *pd = ((pps->r & 0b11111000) << 8) | ((pps->g & 0b11111100) << 3) | (pps->b >> 3);
      } else if (a == 0) {
      } else {
        uint8_t dr = ((*pd) >> 8) & 0b11111000;
        uint8_t dg = ((*pd) >> 3) & 0b11111100;
        uint8_t db = ((*pd) << 3) & 0b11111000;

        RGB_BLEND(pps->r, pps->g, pps->b, dr, dg, db, a)

        *pd = ((dr >> 3) << 11) | ((dg >> 2) << 5) | (db >> 3);
      }

      ps += src_step;
      pd++;
    } while (--count);
  }

  void P_P(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step) {
    uint8_t *ps = (uint8_t *)src->data;        // destination pointer
    uint8_t *pd = (uint8_t *)dest->data;        // destination pointer

    do {
      if (*ps != src->transparent_index) {
        *pd = *ps;
      }
      pd++;
      ps++;
    } while (--count);
  }


}
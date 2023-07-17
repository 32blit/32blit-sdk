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

  __attribute__((always_inline)) inline uint32_t alpha(uint32_t a1, uint32_t a2) {
    return ((a1 + 1) * (a2 + 1)) >> 8;
  }

  __attribute__((always_inline)) inline uint32_t alpha(uint32_t a1, uint32_t a2, uint32_t a3) {
    return ((a1 + 1) * (a2 + 1) * (a3 + 1)) >> 16;
  }

  __attribute__((always_inline)) inline uint8_t blend(uint8_t s, uint8_t d, uint8_t a) {
    return d + ((a * (s - d) + 127) >> 8);
  }

  __attribute__((always_inline)) inline void blend_rgba_rgb(const Pen *s, uint8_t *d, uint8_t a, uint32_t c) {
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
      d = (uint8_t*)d32;
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
      *d = s32 & 0xff; d++;
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
      d = (uint8_t*)d32;
      while (d < de) {
        *d = (s32 & 0xff); s32 >>= 8; d++;
      }
    }
  }

  __attribute__((always_inline)) inline uint16_t pack_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
  }

  __attribute__((always_inline)) inline void unpack_rgb565(uint16_t rgb565, uint8_t &r, uint8_t &g, uint8_t &b) {
    r =  rgb565        & 0x1F; r = r << 3;
    g = (rgb565 >>  5) & 0x3F; g = g << 2;
    b = (rgb565 >> 11) & 0x1F; b = b << 3;
  }

  __attribute__((always_inline)) inline void blend_rgba_rgb565(const Pen *s, uint8_t *d, uint8_t a, uint32_t c) {
    auto *d16 = (uint16_t *)d;
    uint8_t r, g, b;

    if (c == 1) {
      // fast case for single pixel draw
      unpack_rgb565(*d16, r, g, b);
      *d16 = pack_rgb565(blend(s->r, r, a), blend(s->g, g, a), blend(s->b, b, a));
      return;
    }

    // align
    auto de = d16 + c;
    if (uintptr_t(d) & 0b10) {
      unpack_rgb565(*d16, r, g, b);
      *d16++ = pack_rgb565(blend(s->r, r, a), blend(s->g, g, a), blend(s->b, b, a));
    }

    // destination is now aligned
    uint32_t *d32 = (uint32_t*)d16;

    // copy four bytes at a time until we have fewer than four bytes remaining
    uint32_t c32 = uint32_t(de - d16) >> 1;
    while (c32--) {
      uint8_t r2, g2, b2;

      unpack_rgb565(*d32, r, g, b);
      unpack_rgb565(*d32 >> 16, r2, g2, b2);

      *d32++ = pack_rgb565(blend(s->r, r, a), blend(s->g, g, a), blend(s->b, b, a))
             | pack_rgb565(blend(s->r, r2, a), blend(s->g, g2, a), blend(s->b, b2, a)) << 16;
    }

    // copy the trailing bytes as needed
    d16 = (uint16_t*)d32;
    if (d16 < de) {
      unpack_rgb565(*d16, r, g, b);
      *d16 = pack_rgb565(blend(s->r, r, a), blend(s->g, g, a), blend(s->b, b, a));
    }
  }

  __attribute__((always_inline)) inline void copy_rgba_rgb565(const Pen* s, uint8_t *d, uint32_t c) {
    auto *d16 = (uint16_t *)d;
    uint16_t s16 = pack_rgb565(s->r, s->g, s->b);

    if (c == 1) {
      // fast case for single pixel draw
      *d16 = s16;
      return;
    }

    // align
    auto de = d16 + c;
    if (uintptr_t(d) & 0b10)
      *d16++ = s16;

    // destination is now aligned
    uint32_t *d32 = (uint32_t*)d16;

    // create packed 32bit source
    uint32_t s32 = s16 | (s16 << 16);

    // copy four bytes at a time until we have fewer than four bytes remaining
    uint32_t c32 = uint32_t(de - d16) >> 1;
    while (c32--)
      *d32++ = s32;

    // copy the trailing bytes as needed
    d16 = (uint16_t*)d32;
    if (d16 < de)
      *d16 = s16;
  }

  void RGBA_RGBA(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    if(!pen->a) return;

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
    if(!pen->a) return;

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

  void RGBA_RGB565(const Pen* pen, const Surface* dest, uint32_t off, uint32_t c) {
    if(!pen->a) return;

    uint8_t* d = dest->data + (off * 2);
    uint8_t* m = dest->mask ? dest->mask->data + off : nullptr;

    uint16_t a = alpha(pen->a, dest->alpha);
    if (!m) {
      // no mask
      if (a >= 255) {
        // no alpha, just copy
        copy_rgba_rgb565(pen, d, c);
      }
      else {
        // alpha, blend
        blend_rgba_rgb565(pen, d, a, c);
      }
    } else {
      // mask enabled, slow blend
      do {
        uint16_t ma = alpha(a, *m++);
        blend_rgba_rgb565(pen, d, ma, 1);
        d += 2;
      } while (--c);
    }
  }

  void P_P(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt) {
    uint8_t* d = dest->data + off;
    uint8_t transparent = dest->transparent_index;

    do {
      if (pen->a != transparent) {
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
    uint8_t* s = src->palette ? src->data + soff : src->data + (soff * src->pixel_stride);
    uint8_t* d = dest->data + (doff * 4);
    uint8_t* m = dest->mask ? dest->mask->data + doff : nullptr;

    do {
      Pen *pen = src->palette ? &src->palette[*s] : (Pen *)s;

      uint16_t a = src->format == PixelFormat::RGB ? 0 : pen->a;
      a = m ? alpha(a, *m++, dest->alpha) : alpha(a, dest->alpha);

      if (a >= 255) {
        *d++ = pen->r; *d++ = pen->g; *d++ = pen->b; d++;
      } else if (a > 1) {
        *d = blend(pen->r, *d, a); d++;
        *d = blend(pen->g, *d, a); d++;
        *d = blend(pen->b, *d, a); d++;
        *d = blend(pen->a, *d, a); d++;
      }else{
        d += 4;
      }

      s += src->pixel_stride * src_step;
    } while (--cnt);
  }

  void RGBA_RGB(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt, int32_t src_step) {
    uint8_t* s = src->palette ? src->data + soff : src->data + (soff * src->pixel_stride);
    uint8_t* d = dest->data + (doff * 3);
    uint8_t* m = dest->mask ? dest->mask->data + doff : nullptr;

    do {
      Pen *pen = src->palette ? &src->palette[*s] : (Pen *)s;

      uint16_t a = src->format == PixelFormat::RGB ? 255 : pen->a;
      a = m ? alpha(a, *m++, dest->alpha) : alpha(a, dest->alpha);

      if (a >= 255) {
        *d++ = pen->r; *d++ = pen->g; *d++ = pen->b;
      } else if (a > 1) {
        *d = blend(pen->r, *d, a); d++;
        *d = blend(pen->g, *d, a); d++;
        *d = blend(pen->b, *d, a); d++;
      }else{
        d += 3;
      }

      s += (src->pixel_stride) * src_step;
    } while (--cnt);
  }

  void RGBA_RGB565(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt, int32_t src_step) {
    uint8_t* s = src->palette ? src->data + soff : src->data + (soff * src->pixel_stride);
    uint8_t* d = dest->data + (doff * 2);
    uint8_t* m = dest->mask ? dest->mask->data + doff : nullptr;

    auto d16 = (uint16_t *)d;

    do {
      Pen *pen = src->palette ? &src->palette[*s] : (Pen *)s;

      uint16_t a = src->format == PixelFormat::RGB ? 255 : pen->a;
      a = m ? alpha(a, *m++, dest->alpha) : alpha(a, dest->alpha);

      if (a >= 255) {
        *d16++ = pack_rgb565(pen->r, pen->g, pen->b);
      } else if (a > 1) {
        uint8_t r, g, b;
        unpack_rgb565(*d16, r, g, b);
        *d16++ = pack_rgb565(blend(pen->r, r, a), blend(pen->g, g, a), blend(pen->b, b, a));
      }else{
        d16++;
      }

      s += (src->pixel_stride) * src_step;
    } while (--cnt);
  }

  void P_P(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt, int32_t src_step) {
    uint8_t *s = src->data + soff;
    uint8_t *d = dest->data + doff;
    uint8_t transparent = dest->transparent_index;

    do {
      if (*s != transparent) {
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

  Pen get_pen_rgb(const Surface *surf, uint32_t offset) {
    auto ptr = surf->data + offset * 3;
    return {ptr[0], ptr[1], ptr[2]};
  }

  Pen get_pen_rgba(const Surface *surf, uint32_t offset) {
    auto ptr = surf->data + offset * 4;
    return *(Pen *)ptr;
  }

  Pen get_pen_p(const Surface *surf, uint32_t offset) {
    auto ptr = surf->data + offset;
    return surf->palette[*ptr];
  }

  Pen get_pen_m(const Surface *surf, uint32_t offset) {
    auto ptr = surf->data + offset;
    return {*ptr}; // mask is just alpha
  }

  Pen get_pen_rgb565(const Surface *surf, uint32_t offset) {
    auto ptr = surf->data + offset * 2;

    auto rgb565 = *(uint16_t *)ptr;
    uint8_t r, g, b;
    unpack_rgb565(rgb565, r, g, b);
    return {r, g, b};
  }
}

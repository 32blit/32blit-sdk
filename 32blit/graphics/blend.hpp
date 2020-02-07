#pragma once

#include <stdint.h>

namespace blit {
  struct Surface;

  // blends the supplied pen onto a span of pixels in the destination surface
  // supports pen alpha, global alpha, and mask alpha where needed
  typedef void(*PenBlendFunc)(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt);

  // blends the pixel data in the source surface onto a span of pixels in the 
  // destination surface
  // supports source alpha, global alpha, and mask alpha where needed
  typedef void(*BlitBlendFunc)(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt);

  extern void RGBA_RGBA(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt);
  extern void RGBA_RGB(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt);
  extern void P_P(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt);
  extern void M_M(const Pen* pen, const Surface* dest, uint32_t off, uint32_t cnt);

  extern void RGBA_RGBA(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt);
  extern void RGBA_RGB(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt);
  extern void P_P(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt);
  extern void M_M(const Surface* src, uint32_t soff, const Surface* dest, uint32_t doff, uint32_t cnt);

}
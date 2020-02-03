#pragma once

#include <stdint.h>

namespace blit {
  struct Surface;

  // blend a span of pixels from either a source colour or mask value
  typedef void(*blend_span_func)(uint8_t *pen, Surface *dest, uint32_t offset, uint16_t count);

  // blend a span of pixels from a source surface
  typedef void(*BlendBlitFunc)(Surface *src, int32_t src_offset, Surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);

  extern void RGBA_RGB(uint8_t *pen, Surface *dest, uint32_t offset, uint16_t count);
  extern void RGBA_RGBA(uint8_t *pen, Surface *dest, uint32_t offset, uint16_t count);
  extern void RGBA_RGB(Surface *src, int32_t src_offset, Surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);
  extern void RGBA_RGB565(uint8_t *pen, Surface *dest, uint32_t offset, uint16_t count);
  extern void RGBA_RGB565(Surface *src, int32_t src_offset, Surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);
  extern void RGBA_M(uint8_t *pen, Surface *dest, uint32_t offset, uint16_t count);
  extern void RGBA_P(uint8_t *pen, Surface *dest, uint32_t offset, uint16_t count);
  extern void P_P(uint8_t *pen, Surface *dest, uint32_t offset, uint16_t count);
  extern void P_P(Surface *src, int32_t src_offset, Surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);
  extern void P_RGB(Surface *src, int32_t src_offset, Surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);
  extern void P_RGBA(Surface *src, int32_t src_offset, Surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);
  extern void P_RGB565(Surface *src, int32_t src_offset, Surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);

}
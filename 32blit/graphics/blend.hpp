#pragma once

#include <stdint.h>

namespace blit {
  struct surface;

  // blend a span of pixels from either a source colour or mask value
  typedef void(*blend_span_func)(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count);

  // blend a span of pixels from a source surface
  typedef void(*blend_blit_func)(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);

  extern void RGBA_RGB(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count);
  extern void RGBA_RGBA(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count);
  extern void RGBA_RGB(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);
  extern void RGBA_RGB565(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count);
  extern void RGBA_RGB565(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);
  extern void RGBA_M(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count);
  extern void RGBA_P(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count);
  extern void P_P(uint8_t *pen, surface *dest, uint32_t offset, uint16_t count);
  extern void P_P(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);
  extern void P_RGB(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);
  extern void P_RGBA(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);
  extern void P_RGB565(surface *src, int32_t src_offset, surface *dest, int32_t dest_offset, uint16_t count, int16_t src_step);

}
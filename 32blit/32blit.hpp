#pragma once

#include "engine/engine.hpp"
#include "engine/utility.hpp"
#include "engine/input.hpp"
#include "engine/timer.hpp"
#include "graphics/blend.hpp"
#include "graphics/surface.hpp"
#include "graphics/sprite.hpp"
#include "graphics/tilemap.hpp"
#include "graphics/font.hpp"
#include "types/vec3.hpp"
#include "types/mat4.hpp"
#include "types/mat3.hpp"
#include "types/vec2.hpp"
#include "types/map.hpp"
#include "types/point.hpp"
#include "types/rect.hpp"
#include "types/size.hpp"

#ifndef M_PI
  #define M_PI           3.14159265358979323846f  /* pi */
#endif


#ifdef _WIN32
#define __attribute__(A)
extern const uint8_t itcm_text_start;
extern const uint8_t itcm_text_end;
extern const uint8_t itcm_data;
#endif


#define DTCM  __attribute__((section(".dtcm")));
#define ITCM  __attribute__((section(".itcm")));
#define SRAM1 __attribute__((section(".sram1")));
#define SRAM2 __attribute__((section(".sram2")));
#define SRAM3 __attribute__((section(".sram3")));


#pragma once

#include "engine/engine.hpp"
#include "engine/output.hpp"
#include "engine/input.hpp"
#include "audio/audio.hpp"
#include "engine/timer.hpp"
#include "engine/tweening.hpp"
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

#undef M_PI
#define M_PI           3.14159265358979323846f  /* pi */

constexpr float math_pi = 3.14159265358979323846f;


#ifdef TARGET_32BLIT_HW
#define __SECTION__(S) __attribute__((section(S)))
#else
#define __SECTION__(S)
#endif

#if _WIN32
#define __attribute__(A)
extern const uint8_t itcm_text_start;
extern const uint8_t itcm_text_end;
extern const uint8_t itcm_data;
#endif

#define DTCM  __SECTION__(".dtcm");
#define ITCM  __SECTION__(".itcm");
#define SRAM1 __SECTION__(".sram1")));
#define SRAM2 __SECTION__(".sram2")));
#define SRAM3 __SECTION__(".sram3")));

#pragma once

#ifdef TARGET_32BLIT_HW
#define blit_fast_code(fn) __attribute__((section(".itcm." #fn))) fn
#define blit_no_inline_fast_code(fn) __attribute__((noinline)) blit_fast_code(fn)
#elif defined(PICO_BUILD)
#include <pico.h>

#define blit_fast_code(fn) __not_in_flash_func(fn)
#define blit_no_inline_fast_code(fn) __no_inline_not_in_flash_func(fn)
#else
#define blit_fast_code(fn) fn
#define blit_no_inline_fast_code(fn) fn
#endif

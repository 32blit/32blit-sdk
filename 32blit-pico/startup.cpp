#include "engine/engine.hpp"
#include "engine/api_private.hpp"

#include "config.h"

#ifndef BLIT_BOARD_PIMORONI_PICOVISION
[[gnu::section(".uninitialized_data.framebuffer")]]
static uint16_t screen_fb[FRAMEBUFFER_SIZE];
#endif

extern void init();
extern void update(uint32_t time);
extern void render(uint32_t time);

// override terminate handler to save ~20-30k
namespace __cxxabiv1 {
  std::terminate_handler __terminate_handler = std::abort;
}

extern "C" bool do_init() {
#ifndef IGNORE_API_VERSION
  if(blit::api.version_major != blit::api_version_major)
    return false;

  if(blit::api.version_minor < blit::api_version_minor)
    return false;
#endif

  // preinit/init funcs (based on pico-sdk runtime.c)
  // we're not calling the preinit funcs on RP2350 as they're all low-level init that should've been done by the loader
  // (on RP2040 they init the ROM functions, so we do need to call them)
#ifndef PICO_RP2350
  // Start and end points of the constructor list,
  // defined by the linker script.
  extern void (*__preinit_array_start)();
  extern void (*__preinit_array_end)();
  // Call each function in the list.
  // We have to take the address of the symbols, as __preinit_array_start *is*
  // the first function pointer, not the address of it.
  for (void (**p)(void) = &__preinit_array_start; p < &__preinit_array_end; ++p) {
    (*p)();
  }
#endif

  // Start and end points of the constructor list,
  // defined by the linker script.
  extern void (*__init_array_start)(void);
  extern void (*__init_array_end)(void);

  // Call each function in the list.
  // We have to take the address of the symbols, as __init_array_start *is*
  // the first function pointer, not the address of it.
  for (void (**p)(void) = &__init_array_start; p < &__init_array_end; ++p) {
    (*p)();
  }

  blit::update = update;
  blit::render = render;

#ifndef BLIT_BOARD_PIMORONI_PICOVISION
  blit::api.set_framebuffer((uint8_t *)screen_fb, sizeof(screen_fb), {DISPLAY_WIDTH, DISPLAY_HEIGHT});
#endif

  blit::set_screen_mode(blit::ScreenMode::lores);

  init();

  return true;
}

extern "C" void do_render(uint32_t time) {
  blit::screen.data = blit::api.get_screen_data();
  render(time);
}

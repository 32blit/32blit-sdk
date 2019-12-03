
#include "32blit.hpp"

extern char __ltdc_start;
extern void blit_tick();
extern void blit_init();
extern void blit_swap();
extern void blit_flip();
extern void set_screen_mode(blit::screen_mode new_mode);
extern void blit_clear_framebuffer();
extern void blit_update_vibration();
extern void blit_update_led();
extern void blit_process_input();
extern void init();
extern void update(uint32_t time);
extern void render(uint32_t time);
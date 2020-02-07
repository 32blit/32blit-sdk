#undef UNUSED
#define UNUSED(x) ((void)(__typeof__(x))(x)) // suppress "UNUSED" warnings

#include "32blit.hpp"
#include "fatfs.h"

// Functions defined by user code files
extern void init();
extern void update(uint32_t time);
extern void render(uint32_t time);

// SD storage
extern char *get_fr_err_text(FRESULT err);
extern bool blit_sd_detected();

// LTDC and framebuffer handling
extern char __ltdc_start;
extern void blit_swap();
extern void blit_flip();
extern void set_screen_mode(blit::ScreenMode new_mode);
extern void blit_clear_framebuffer();

// Blit setup and main loop
extern void blit_tick();
extern void blit_init();

// IO
extern void blit_update_vibration();
extern void blit_update_led();
extern void blit_process_input();

// Audio
extern void blit_enable_amp();

// Switching execution
extern void blit_switch_execution(void);

void blit_menu_update(uint32_t time);
void blit_menu_render(uint32_t time);
void blit_menu();

extern void blit_enable_ADC();
extern void blit_disable_ADC();

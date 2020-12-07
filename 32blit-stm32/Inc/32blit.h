#undef UNUSED
#define UNUSED(x) ((void)(__typeof__(x))(x)) // suppress "UNUSED" warnings

#include "32blit.hpp"
#include "fatfs.h"
#include "persistence.h"

extern bool is_beta_unit;

// Functions defined by user code files
extern void init();
extern void update(uint32_t time);
extern void render(uint32_t time);

// SD storage
extern char *get_fr_err_text(FRESULT err);
extern bool blit_sd_detected();
extern bool blit_sd_mounted();

extern void render_yield();

// Blit setup and main loop
extern void blit_tick();
extern void blit_init();

// IO
extern void blit_update_vibration();
extern void blit_update_led();
extern void blit_process_input();
extern void blit_i2c_tick();

// Switching execution.
// Address is relative to the start of flash, ignored if switching to firmware
extern void blit_switch_execution(uint32_t address, bool force_game);
extern bool blit_user_code_running();
extern "C" void blit_reset_with_error();
extern void blit_enable_user_code();
extern void blit_disable_user_code();

void blit_menu_update(uint32_t time);
void blit_menu_render(uint32_t time);
void blit_menu();

extern void blit_enable_ADC();
extern void blit_disable_ADC();

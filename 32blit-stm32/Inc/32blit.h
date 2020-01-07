
#include "32blit.hpp"
#include "fatfs.h"

#define DAC_BUFFER_SIZE 4000
#define DAC_DMA_COMPLETE 2
#define DAC_DMA_HALF_COMPLETE 1

// Functions defined by user code files
extern void init();
extern void update(uint32_t time);
extern void render(uint32_t time);

// SD storage
bool blit_mount_sd(char label[12], uint32_t &totalspace, uint32_t &freespace);
bool blit_open_file(FIL &file, const char *filename);
extern char *get_fr_err_text(FRESULT err);
extern bool blit_sd_detected();

// LTDC and framebuffer handling
extern char __ltdc_start;
extern void blit_swap();
extern void blit_flip();
extern void set_screen_mode(blit::screen_mode new_mode);
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
//extern uint32_t blit_update_dac(FIL *audio_file);

void blit_menu_update(uint32_t time);
void blit_menu_render(uint32_t time);
void blit_menu();
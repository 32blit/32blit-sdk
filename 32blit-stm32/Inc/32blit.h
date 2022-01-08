#pragma once

#undef UNUSED
#define UNUSED(x) ((void)(__typeof__(x))(x)) // suppress "UNUSED" warnings

#include "32blit.hpp"
#include "persistence.h"
#include "executable.hpp"

extern bool is_beta_unit;

// Functions defined by user code files
void init();
void update(uint32_t time);
void render(uint32_t time);

// SD storage
bool blit_sd_detected();
bool blit_sd_mounted();

void render_yield();

// Blit setup and main loop
void blit_tick();
void blit_init();

// IO
void blit_update_volume();
void blit_update_vibration();
void blit_update_led();
void blit_process_input();

// Switching execution.
// Address is relative to the start of flash, ignored if switching to firmware
bool blit_switch_execution(uint32_t address, bool force_game);
bool blit_user_code_running();
extern "C" void blit_reset_with_error();
void blit_enable_user_code();
void blit_disable_user_code();
RawMetadata *blit_get_running_game_metadata();

void blit_menu_update(uint32_t time);
void blit_menu_render(uint32_t time);
void blit_menu();

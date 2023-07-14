#pragma once
#include <cstdint>

#include "engine/api_private.hpp"
#include "config.h"

// height rounded up to handle the 135px display
// this is in bytes
static const int lores_page_size = (DISPLAY_WIDTH / 2) * ((DISPLAY_HEIGHT + 1) / 2) * 2;

extern blit::ScreenMode cur_screen_mode;
extern uint16_t screen_fb[];

void init_display();
void update_display(uint32_t time);

void init_display_core1();
void update_display_core1();

bool display_render_needed();

bool display_mode_supported(blit::ScreenMode new_mode, const blit::SurfaceTemplate &new_surf_template);

void display_mode_changed(blit::ScreenMode new_mode, blit::SurfaceTemplate &new_surf_template);

blit::SurfaceInfo &set_screen_mode(blit::ScreenMode mode);
bool set_screen_mode_format(blit::ScreenMode new_mode, blit::SurfaceTemplate &new_surf_template);

void set_screen_palette(const blit::Pen *colours, int num_cols);

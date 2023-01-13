#pragma once
#include <cstdint>

#include "engine/api_private.hpp"

void init_display();
void update_display(uint32_t time);

void init_display_core1();
void update_display_core1();

bool display_render_needed();

blit::SurfaceInfo &set_screen_mode(blit::ScreenMode mode);
bool set_screen_mode_format(blit::ScreenMode new_mode, blit::SurfaceTemplate &new_surf_template);

void set_screen_palette(const blit::Pen *colours, int num_cols);

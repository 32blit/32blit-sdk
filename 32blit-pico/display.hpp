#pragma once
#include <cstdint>

#include "engine/api_private.hpp"
#include "config.h"

extern blit::SurfaceInfo cur_surf_info;
extern blit::ScreenMode cur_screen_mode;

extern bool fb_double_buffer;

#if defined(BUILD_LOADER) || defined(BLIT_BOARD_PIMORONI_PICOVISION)
extern uint16_t *screen_fb;
#else
extern uint16_t screen_fb[];
#endif

int get_display_page_size();
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

void set_framebuffer(uint8_t *data, uint32_t max_size);

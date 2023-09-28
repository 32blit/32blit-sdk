#include "display.hpp"

#include "config.h"

void init_display() {
}

void update_display(uint32_t time) {
}

void init_display_core1() {
}

void update_display_core1() {
}

bool display_render_needed() {
  return false;
}

bool display_mode_supported(blit::ScreenMode new_mode, const blit::SurfaceTemplate &new_surf_template) {
  return false;
}

void display_mode_changed(blit::ScreenMode new_mode, blit::SurfaceTemplate &new_surf_template) {
}

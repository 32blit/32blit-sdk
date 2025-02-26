#pragma once

void set_render_overlay_enabled(bool enabled);

void set_overlay_message(std::string_view text);
void set_overlay_progress(uint32_t value, uint32_t total);

void overlay_try_render(bool force);

#include "display.hpp"

#include "engine/engine.hpp"

using namespace blit;

static void(*orig_render)(uint32_t) = nullptr;

static std::string message;
static uint32_t progress_value = 0, progress_total = 0;

static void overlay_render(uint32_t time) {
  // TODO: don't want to do this if we add a menu to avoid the problems the stm32 firmware has...
  if(orig_render)
    orig_render(time);

  if(!message.empty() || progress_total) {
    screen.pen = Pen(0, 0, 0, 150);
    screen.rectangle(Rect(0, screen.bounds.h - 25, screen.bounds.w, 25));
    screen.pen = Pen(255, 255, 255);
    screen.text(message, minimal_font, Point(5, screen.bounds.h - 20));
    uint32_t progress_width = float(progress_value) / progress_total * (screen.bounds.w - 10);
    screen.rectangle(Rect(5, screen.bounds.h - 10, progress_width, 5));
  }
}

void set_render_overlay_enabled(bool enabled) {
  if(enabled && blit::render != overlay_render) {
    orig_render = blit::render;
    blit::render = overlay_render;
  } else if(!enabled && orig_render) {
    blit::render = orig_render;
    orig_render = nullptr;
  }
}

void set_overlay_message(std::string_view text) {
  message = text;
}

void set_overlay_progress(uint32_t value, uint32_t total) {
  progress_value = value;
  progress_total = total;
}

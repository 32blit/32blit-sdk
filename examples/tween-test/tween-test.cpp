#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "tween-test.hpp"
#include "graphics/color.hpp"

using namespace blit;

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;

blit::tween tween_bounce;
blit::tween tween_colour;

void init() {
  blit::set_screen_mode(blit::screen_mode::hires);

  tween_bounce.init(tween_sine, 200.0f, 0.0f, 5000, -1);
  tween_bounce.start();

  tween_colour.init(tween_sine, 0.0f, 1.0f, 10000, -1);
  tween_colour.start();
}

int tick_count = 0;
void render(uint32_t time_ms) {
  char text_buffer[60];
  fb.pen(rgba(20, 30, 40));
  fb.clear();

  fb.pen(blit::hsv_to_rgba(tween_colour.value, 1.0f, 1.0f));
  fb.circle(point(160, 20+tween_bounce.value), 20);

  fb.pen(rgba(255, 255, 255));
  sprintf(text_buffer, "Value: %f", tween_bounce.value);
  fb.text(text_buffer, &minimal_font[0][0], point(175, 35+tween_bounce.value));
}

void update(uint32_t time_ms) {
  blit::update_tweens(time_ms);
}
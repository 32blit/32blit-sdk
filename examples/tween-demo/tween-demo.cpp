#include <string>

#include "tween-demo.hpp"

using namespace blit;

Tween tween;

struct TweenInfo {
  Tween::TweenFunction func;
  const char *name;
};

static const int num_tween_funcs = 5;
TweenInfo tween_funcs[num_tween_funcs]{
  {tween_linear, "Linear"},
  {tween_ease_in_quad, "Ease in quad"},
  {tween_ease_out_quad, "Ease out quad"},
  {tween_ease_in_out_quad, "Ease in out quad"},
  {tween_sine, "Sine"},
};

int current_tween_func = 0;

void init() {
  set_screen_mode(ScreenMode::hires);

  tween.init(tween_linear, 0.0f, 200.0f, 5000, -1);
  tween.start();
}

void render(uint32_t time_ms) {
  screen.pen = Pen(20, 30, 40);
  screen.clear();

  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));

  screen.pen = Pen(0, 0, 0);
  screen.text(std::string("Tween demo - ") + tween_funcs[current_tween_func].name, minimal_font, Point(5, 4));

  screen.pen = Pen(255, 255, 255);
  screen.circle(Point(310, 24 + tween.value), 5);

  // graph curve
  const int graph_y = 24, graph_w = 280, graph_h = 200;
  int prev_y = tween.function(0, 0, graph_h, 200);

  for(int x = 1; x < graph_w; x++) {
    int y = tween.function(x, 0, graph_h, graph_w);

    screen.line(Point(x + 20 - 1, prev_y + graph_y), Point(x + 20, y + graph_y));
    prev_y = y;
  }

  screen.pen = Pen(127, 127, 127);

  screen.h_span(Point(20, graph_y + graph_h), graph_w);
  screen.v_span(Point(20, graph_y), graph_h);
}

void update(uint32_t time_ms) {
  if(buttons.released & Button::DPAD_LEFT) {
    current_tween_func = current_tween_func == 0 ? num_tween_funcs - 1 : current_tween_func - 1;

    tween.function = tween_funcs[current_tween_func].func;
    tween.start();
  } else if(buttons.released & Button::DPAD_RIGHT) {
    current_tween_func = current_tween_func + 1 == num_tween_funcs ? 0 : current_tween_func + 1;

    tween.function = tween_funcs[current_tween_func].func;
    tween.start();
  }
}
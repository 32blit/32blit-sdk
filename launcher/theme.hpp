#pragma once

#include "32blit.hpp"

struct Theme {
  blit::Pen color_background;
  blit::Pen color_overlay;
  blit::Pen color_text;
  blit::Pen color_accent;
};

extern Theme theme;

extern void init_theme();
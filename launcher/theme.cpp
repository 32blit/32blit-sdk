#include "theme.hpp"

using namespace blit;

Theme theme = {
  Pen(0, 0, 0, 255),
  Pen(255, 255, 255, 10),
  Pen(180, 180, 220, 255),
  Pen(0, 255, 0, 255)
};

void init_theme(void) {
  if(!read_save(theme)) {
    write_save(theme);
  }
}
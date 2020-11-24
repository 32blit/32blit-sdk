#include <cstring>

#include "saves.hpp"

using namespace blit;

// The data you want to save. Should not contain any pointers or other data that can't be written to a file as-is.
struct MyGameSave {
  char name[10];
  int32_t score;
};

MyGameSave save_data;

void init() {
  // Attempt to load the first save slot.
  if(read_save(save_data)) {
    // Loaded sucessfully!
  } else {
    // No save file or it failed to load, set up some defaults.
    strncpy(save_data.name, "My Name", 10);
    save_data.score = 0;
  }
}

void render(uint32_t time_ms) {
  screen.pen = Pen(20, 30, 40);
  screen.clear();

  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));

  screen.pen = Pen(0, 0, 0);
  screen.text("Saves", minimal_font, Point(5, 4));

  screen.pen = Pen(255, 255, 255);
  char buf[100];
  snprintf(buf, 100, "Hello %s\n\nHold A to win!\n\nCurrent score: %i", save_data.name, save_data.score);
  screen.text(buf, minimal_font, Point(screen.bounds.w / 2, screen.bounds.h / 2), false, TextAlign::center_center);
}

void update(uint32_t time) {
  // Give away some points
  if(buttons & Button::A) {
    save_data.score++;
  }

  // Save all those points we gave away
  if(buttons.released & Button::A) {
    write_save(save_data);
  }
}
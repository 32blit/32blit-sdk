#pragma once

#include "32blit.hpp"

#define ROW_HEIGHT 14

using namespace blit;

enum class SortBy {
  name,
  size
};

enum class Screen {
  main,
  credits,
  screenshot
};

enum class GameType {
  game,
  file,
  screenshot
};

struct GameInfo {
  GameType type;

  std::string title;
  uint32_t size, checksum = 0;

  std::string filename, ext;
};

struct DirectoryInfo {
  std::string name;
  int x, w;
};

int selected_menu_item;

class AutoRepeat {
  public:
    AutoRepeat(uint32_t repeat_time=200, uint32_t hold_time=1000) {
      this->repeat_time = repeat_time;
      this->hold_time = hold_time;
    }
    bool next(uint32_t time, bool state) {
      bool changed = state != last_state;
      last_state = state;

      if(changed) {
        if(state) {
          pressed_time = time;
          pressed = true;
          last_time = time;
          return true;
        }
        else {
          pressed_time = 0;
          pressed = false;
          last_time = 0;
        }
      }
      // Shortcut for no auto-repeat
      if(repeat_time == 0) return false;

      if(pressed) {
        uint32_t repeat_rate = repeat_time;
        if(hold_time > 0 && time - pressed_time > hold_time) {
          repeat_rate /= 3;
        }
        if(time - last_time > repeat_rate) {
          last_time = time;
          return true;
        }
      }

      return false;
    }
  private:
    uint32_t repeat_time;
    uint32_t hold_time;
    bool pressed = false;
    bool last_state = false;
    uint32_t pressed_time = 0;
    uint32_t last_time = 0;
};

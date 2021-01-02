#pragma once

#include "32blit.hpp"

#define ROW_HEIGHT 12

using namespace blit;

enum class SortBy {
  name,
  size
};

struct GameInfo {
  std::string title;
  uint32_t size, checksum = 0;

  std::string filename, ext;
};

struct DirectoryInfo {
  std::string name;
  int x, w;
};

int selected_menu_item;
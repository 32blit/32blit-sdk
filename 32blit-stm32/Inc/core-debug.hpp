#pragma once

#include <string>
#include <vector>

namespace debug {

  extern std::vector<std::string> messages;
  extern void debug(const char *p);
  extern void render();
}
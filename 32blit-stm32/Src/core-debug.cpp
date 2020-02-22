
#include "core-debug.hpp"

#include "32blit.h"

using namespace blit;

namespace debug {

  std::vector<std::string> messages;

  void debug(const char *p) {
    std::string message = p;

    if(messages.size() > 20) {
      messages.erase(messages.begin());
    }

    messages.push_back(message);
  }

  void render() {
    if(messages.size() > 0) {
      screen.pen = Pen(0, 0, 0, 200);
      screen.rectangle(Rect(10, 10, 300, 220));
      screen.pen = Pen(255, 255, 255, 200);
      uint8_t i = 0;
      for(auto message : messages) {
        screen.text(message, minimal_font, Point(12, 12 + i * 10));
        i++;
      }
    }
  }

}
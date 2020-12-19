#include <cstring>

#include "multiplayer.hpp"

#include "engine/multiplayer.hpp"

using namespace blit;

std::vector<std::string> messages;

void on_message(const uint8_t *data, uint16_t len) {
  messages.push_back(std::string((const char *)data, len));
}

/* setup */
void init() {

  message_received = on_message;
  enable_multiplayer();
}

void render(uint32_t time_ms) {
  screen.pen = Pen(0, 0, 0);
  screen.clear();

  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));
  screen.pen = Pen(0, 0, 0);
  screen.text("Multiplayer Example", minimal_font, Point(5, 4));

  screen.pen = Pen(64, 64, 64);
  screen.text("Press A to send message.", minimal_font, Point(20, 16));

  int y = 30;
  for(auto &msg : messages) {
    screen.text(msg, minimal_font, Point(0, y));
    y += 10;
  }
} 

void update(uint32_t time_ms) {
  static uint32_t last_buttons = buttons;

  bool button_pressed = (buttons & Button::A) && !(last_buttons & Button::A);

  if(button_pressed) {
    auto msg = "This is a message!";
    send_message((uint8_t *)msg, strlen(msg));
  }

  last_buttons = buttons;
}
  

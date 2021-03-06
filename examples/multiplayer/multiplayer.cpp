#include <cstring>
#include <deque>

#include "multiplayer.hpp"

#include "engine/multiplayer.hpp"

using namespace blit;

std::deque<std::string> messages;
unsigned sent_count = 0;
unsigned recv_count = 0;
static constexpr int max_messages = 9;

void on_message(const uint8_t *data, uint16_t len) {
  if (messages.size() >= max_messages) {
    messages.pop_front();
  }
  messages.push_back(std::string((const char *)data, len));
  recv_count++;
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
  if(is_multiplayer_connected())
    screen.text("Press A to send message.", minimal_font, Point(screen.bounds.w / 2, 18), true, TextAlign::top_center);
  else
    screen.text("Not connected!", minimal_font, Point(screen.bounds.w / 2, 18), true, TextAlign::top_center);

  char counts[50];
  snprintf(counts, 50, "Sent: %u, Recv: %u", sent_count, recv_count);
  screen.text(counts, minimal_font, Point(screen.bounds.w/2, screen.bounds.h-2), true, TextAlign::bottom_center);

  screen.pen = Pen(96, 96, 96);
  int y = 28 + ((9-(int)messages.size()) * max_messages);
  for(auto &msg : messages) {
    screen.text(msg, minimal_font, Point(5, y));
    y += 9;
  }
}

void update(uint32_t time_ms) {
  if ((buttons.pressed & Button::A) && is_multiplayer_connected()) {
    char message[50];
    snprintf(message, 50, "This is message %u!", sent_count);
    send_message((uint8_t *) message, (uint16_t)strlen(message));
    sent_count++;
  }
}

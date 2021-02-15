#pragma once

namespace multiplayer {
  void init();

  extern bool enabled;

  bool is_connected();
  void set_enabled(bool enabled);
  void send_message(const uint8_t *data, uint16_t length);

  void update();
}

#pragma once

namespace multiplayer {
  void init();

  bool is_connected();
  void send_message(const uint8_t *data, uint16_t length);
}
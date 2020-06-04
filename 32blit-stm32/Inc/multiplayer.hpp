#pragma once

namespace multiplayer {
  void init();

  void send_message(const uint8_t *data, uint16_t length);
}
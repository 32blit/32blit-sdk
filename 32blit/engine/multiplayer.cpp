#include "multiplayer.hpp"
#include "api_private.hpp"

namespace blit {
  void send_message(const uint8_t *data, uint16_t len) {
    api.send_message(data, len);
  }

  void (*&message_received)(const uint8_t *data, uint16_t len) = api.message_received;
}
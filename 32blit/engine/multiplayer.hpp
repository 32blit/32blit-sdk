#include <cstdint>

namespace blit {
  bool is_multiplayer_connected();
  void send_message(const uint8_t *data, uint16_t len);

  extern void (*&message_received)(const uint8_t *data, uint16_t len); 
}
#pragma once
#include <cstdint>

#include "usb.hpp"

class CDCHandshakeCommand final : public CDCCommand {
  void init() override {}
  Status update() override;
};

class CDCUserCommand final : public CDCCommand {
  void init() override;
  Status update() override;
};

void update_multiplayer();
void send_multiplayer_handshake(bool is_reply = false);

bool is_multiplayer_connected();
void set_multiplayer_enabled(bool enabled);
void send_multiplayer_message(const uint8_t *data, uint16_t len);

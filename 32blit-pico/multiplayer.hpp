#pragma once
#include <cstdint>

void update_multiplayer();
void send_multiplayer_handshake(bool is_reply = false);

bool is_multiplayer_connected();
void set_multiplayer_enabled(bool enabled);
void send_multiplayer_message(const uint8_t *data, uint16_t len);

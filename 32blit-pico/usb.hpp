#pragma once

#include <cstdint>

void init_usb();
void update_usb();

void usb_debug(const char *message);

// TODO: separate multiplayer from usb
bool is_multiplayer_connected();
void set_multiplayer_enabled(bool enabled);
void send_multiplayer_message(const uint8_t *data, uint16_t len);

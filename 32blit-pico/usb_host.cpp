#include "usb.hpp"

#include <cstring>

#include "tusb.h"

#include "config.h"

void init_usb() {
  tusb_init();
}

void update_usb() {
  tuh_task();
}

void usb_debug(const char *message) {

}

// multiplayer could be supported with USB host, but we'd need a hub
// (host code is used for hid)
bool is_multiplayer_connected() {
  return false;
}

void set_multiplayer_enabled(bool enabled) {
}

void send_multiplayer_message(const uint8_t *data, uint16_t len) {
}

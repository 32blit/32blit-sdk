#include "usb.hpp"

#include <cstring>

#include "bsp/board.h"
#include "tusb.h"

void init_usb() {
  tusb_init();
}

void update_usb() {
  tud_task();
}

void usb_debug(const char *message) {
  if(!tud_cdc_connected())
    return;

  auto len = strlen(message);
  uint32_t done = tud_cdc_write(message, len);

  while(done < len) {
    tud_task();
    if(!tud_cdc_connected())
      break;

    done += tud_cdc_write(message + done, len - done);
  }
}

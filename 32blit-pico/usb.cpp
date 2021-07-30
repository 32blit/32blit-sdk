#include "usb.hpp"

#include "bsp/board.h"
#include "tusb.h"

void init_usb() {
  tusb_init();
}

void update_usb() {
  tud_task();
}

#pragma once

#include <cstdint>

class CDCCommand {
public:
  enum class Status {
    Done = 0,
    Continue,
    Error
  };

  virtual void init() = 0;
  virtual Status update() = 0;
};

void init_usb();
void update_usb();

bool usb_cdc_connected();
uint16_t usb_cdc_read(uint8_t *data, uint16_t len);
uint32_t usb_cdc_read_available();
void usb_cdc_write(const uint8_t *data, uint16_t len);
void usb_cdc_flush_write();

void usb_cdc_update();

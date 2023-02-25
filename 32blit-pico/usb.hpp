#pragma once

#include <cstdint>

void init_usb();
void update_usb();

void usb_debug(const char *message);

bool usb_cdc_connected();
uint16_t usb_cdc_read(uint8_t *data, uint16_t len);
uint32_t usb_cdc_read_available();
void usb_cdc_write(const uint8_t *data, uint16_t len);
void usb_cdc_flush_write();

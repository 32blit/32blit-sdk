#include "usb.hpp"

#include "tusb.h"

#include "config.h"
#include "file.hpp"
#include "storage.hpp"

// msc
static bool storage_ejected = false;

void tud_mount_cb() {
  storage_ejected = false;
}

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
  (void) lun;

  const char vid[] = USB_VENDOR_STR;
  const char pid[] = USB_PRODUCT_STR " Storage";
  const char rev[] = "1.0";

  memcpy(vendor_id  , vid, strlen(vid));
  memcpy(product_id , pid, strlen(pid));
  memcpy(product_rev, rev, strlen(rev));
}

bool tud_msc_test_unit_ready_cb(uint8_t lun) {
  if(!is_storage_available() || storage_ejected ) {
    tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
    return false;
  }

  return true;
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {
  (void) lun;

  get_storage_size(*block_size, *block_count);
}

bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
  (void) lun;
  (void) power_condition;

  if(load_eject) {
    if (start) {
    } else
      storage_ejected = true;
  }

  return true;
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
  (void) lun;

  return storage_read(lba, offset, buffer, bufsize);
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
  (void) lun;

  return storage_write(lba, offset, buffer, bufsize);
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize) {
  uint16_t resplen = 0;

  switch (scsi_cmd[0]) {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
      // Host is about to read/write etc ... better not to disconnect disk
      resplen = 0;
    break;

    default:
      // Set Sense = Invalid Command Operation
      tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

      // negative means error -> tinyusb could stall and/or response with failed status
      resplen = -1;
    break;
  }

  return resplen;
}

bool tud_msc_is_writable_cb(uint8_t lun) {
  return !get_files_open();
}

// cdc
void init_usb() {
  tusb_init();
}

void update_usb() {
  tud_task();

  usb_cdc_update();
}

bool usb_cdc_connected() {
  // tud_cdc_connected returns false with STM32 USB host
  return tud_ready(); //tud_cdc_connected();
}

uint16_t usb_cdc_read(uint8_t *data, uint16_t len) {
  return tud_cdc_read(data, len);
}

uint32_t usb_cdc_read_available() {
  return tud_cdc_available();
}

void usb_cdc_write(const uint8_t *data, uint16_t len) {
  uint32_t done = tud_cdc_write(data, len);

  while(done < len) {
    tud_task();
    if(!tud_ready())
      break;

    done += tud_cdc_write(data + done, len - done);
  }
}

void usb_cdc_flush_write() {
  tud_cdc_write_flush();
}

#include "usb.hpp"

#include <cstring>

#include "tusb.h"

#include "config.h"
#include "file.hpp"
#include "storage.hpp"

#include "engine/api_private.hpp"

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
  if(storage_ejected) {
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
static bool multiplayer_enabled = false;
static bool peer_connected = false;

static char cur_header[8];
static int header_pos = 0;

static uint16_t mp_buffer_len, mp_buffer_off;
static uint8_t *mp_buffer = nullptr;

static void send_all(const void *buffer, uint32_t len) {
  uint32_t done = tud_cdc_write(buffer, len);

  while(done < len) {
    tud_task();
    if(!tud_ready())
      break;

    done += tud_cdc_write((const char *)buffer + done, len - done);
  }
}

static void send_handshake(bool is_reply = false) {
  uint8_t val = 0;
  if(multiplayer_enabled)
    val = is_reply ? 2 : 1;

  uint8_t buf[]{'3', '2', 'B', 'L', 'M', 'L', 'T','I', val};
  send_all(buf, 9);
  tud_cdc_write_flush();
}

void init_usb() {
  tusb_init();
}

void update_usb() {
  tud_task();

  if(!tud_ready()) { // tud_cdc_connected returns false with STM USB host
    peer_connected = false;
  }

  while(tud_cdc_available()) {
    // match header
    if(header_pos < 8) {
      cur_header[header_pos] = tud_cdc_read_char();

      const char *expected = "32BL";
      if(header_pos >= 4 || cur_header[header_pos] == expected[header_pos])
        header_pos++;
      else
        header_pos = 0;
    } else {

      // get USER packet
      if(mp_buffer) {
        mp_buffer_off += tud_cdc_read(mp_buffer + mp_buffer_off, mp_buffer_len - mp_buffer_off);

        if(mp_buffer_off == mp_buffer_len) {
          if(blit::api.message_received)
            blit::api.message_received(mp_buffer, mp_buffer_len);

          delete[] mp_buffer;
          mp_buffer = nullptr;
          header_pos = 0;
        }
        continue;
      }

      // got header
      if(memcmp(cur_header + 4, "MLTI", 4) == 0) {
        // handshake packet
        peer_connected = tud_cdc_read_char() != 0;

        if(peer_connected)
          send_handshake(true);

        // done
        header_pos = 0;
      } else if(memcmp(cur_header + 4, "USER", 4) == 0) {
        if(tud_cdc_available() < 2)
          break;

        tud_cdc_read(&mp_buffer_len, 2);
        mp_buffer_off = 0;
        mp_buffer = new uint8_t[mp_buffer_len];

      } else {
        printf("got: %c%c%c%c%c%c%c%c\n", cur_header[0], cur_header[1], cur_header[2], cur_header[3], cur_header[4], cur_header[5], cur_header[6], cur_header[7]);
        header_pos = 0;
      }
    }
  }
}

void usb_debug(const char *message) {
  if(!tud_cdc_connected())
    return;

  auto len = strlen(message);
  send_all(message, len);
}

bool is_multiplayer_connected() {
  return multiplayer_enabled && peer_connected;
}

void set_multiplayer_enabled(bool enabled) {
  multiplayer_enabled = enabled;

  if(!enabled)
    send_handshake();
}

void send_multiplayer_message(const uint8_t *data, uint16_t len) {
  if(!peer_connected)
    return;

  uint8_t buf[]{'3', '2', 'B', 'L', 'U', 'S', 'E','R',
    uint8_t(len & 0xFF), uint8_t(len >> 8)
  };
  send_all(buf, 10);

  send_all((uint8_t *)data, len);

  tud_cdc_write_flush();
}

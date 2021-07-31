#include "usb.hpp"

#include <cstring>

#include "bsp/board.h"
#include "tusb.h"

#include "engine/api_private.hpp"

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

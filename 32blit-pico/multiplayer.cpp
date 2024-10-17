#include <cstring>
#include "multiplayer.hpp"

#include "usb.hpp"

#include "engine/api_private.hpp"

static bool multiplayer_enabled = false;
static bool peer_connected = false;

static uint8_t cur_header[8];
static int header_pos = 0;

static uint16_t mp_buffer_len, mp_buffer_off;
static uint8_t *mp_buffer = nullptr;

void send_multiplayer_handshake(bool is_reply) {
  uint8_t val = 0;
  if(multiplayer_enabled)
    val = is_reply ? 2 : 1;

  uint8_t buf[]{'3', '2', 'B', 'L', 'M', 'L', 'T','I', val};
  usb_cdc_write(buf, 9);
  usb_cdc_flush_write();
}

void update_multiplayer() {
  if(!usb_cdc_connected()) {
    peer_connected = false;
  }

  while(usb_cdc_read_available()) {
    // match header
    if(header_pos < 8) {
      usb_cdc_read(cur_header + header_pos, 1);

      const char *expected = "32BL";
      if(header_pos >= 4 || cur_header[header_pos] == expected[header_pos])
        header_pos++;
      else
        header_pos = 0;
    } else {

      // get USER packet
      if(mp_buffer) {
        mp_buffer_off += usb_cdc_read(mp_buffer + mp_buffer_off, mp_buffer_len - mp_buffer_off);

        if(mp_buffer_off == mp_buffer_len) {
          if(blit::api_data.message_received)
            blit::api_data.message_received(mp_buffer, mp_buffer_len);

          delete[] mp_buffer;
          mp_buffer = nullptr;
          header_pos = 0;
        }
        continue;
      }

      // got header
      if(memcmp(cur_header + 4, "MLTI", 4) == 0) {
        // handshake packet
        uint8_t b;
        usb_cdc_read(&b, 1);
        peer_connected = b != 0;

        if(peer_connected)
          send_multiplayer_handshake(true);

        // done
        header_pos = 0;
      } else if(memcmp(cur_header + 4, "USER", 4) == 0) {
        if(usb_cdc_read_available() < 2)
          break;

        usb_cdc_read((uint8_t *)&mp_buffer_len, 2);
        mp_buffer_off = 0;
        mp_buffer = new uint8_t[mp_buffer_len];

      } else {
        printf("got: %c%c%c%c%c%c%c%c\n", cur_header[0], cur_header[1], cur_header[2], cur_header[3], cur_header[4], cur_header[5], cur_header[6], cur_header[7]);
        header_pos = 0;
      }
    }
  }
}

bool is_multiplayer_connected() {
  return multiplayer_enabled && peer_connected;
}

void set_multiplayer_enabled(bool enabled) {
  multiplayer_enabled = enabled;

  if(!enabled)
    send_multiplayer_handshake();
}

void send_multiplayer_message(const uint8_t *data, uint16_t len) {
  if(!peer_connected)
    return;

  uint8_t buf[]{'3', '2', 'B', 'L', 'U', 'S', 'E','R',
    uint8_t(len & 0xFF), uint8_t(len >> 8)
  };
  usb_cdc_write(buf, 10);

  usb_cdc_write((uint8_t *)data, len);

  usb_cdc_flush_write();
}

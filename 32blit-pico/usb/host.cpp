#include "usb.hpp"

#include "multiplayer.hpp"

#include "tusb.h"

#include "config.h"

// hid
#ifdef INPUT_USB_HID

static int hid_report_id = -1;
static uint16_t buttons_offset = 0, num_buttons = 0;
static uint16_t hat_offset = 0xFFFF, stick_offset = 0;

uint32_t hid_gamepad_id = 0;
bool hid_keyboard_detected = false;
uint8_t hid_joystick[2]{0x80, 0x80};
uint8_t hid_hat = 8;
uint32_t hid_buttons = 0;
uint8_t hid_keys[6]{};

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
  uint16_t vid = 0, pid = 0;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  printf("Mount %i %i, %04x:%04x\n", dev_addr, instance, vid, pid);

  auto protocol = tuh_hid_interface_protocol(dev_addr, instance);

  hid_keyboard_detected = protocol == HID_ITF_PROTOCOL_KEYBOARD;

  // don't attempt to use a keyboard/mouse as a gamepad
  if(protocol != HID_ITF_PROTOCOL_NONE) {
    tuh_hid_receive_report(dev_addr, instance);
    return;
  }

  // basic and probably wrong report descriptor parsing
  auto desc_end = desc_report + desc_len;
  auto p = desc_report;

  int report_id = -1;
  int usage_page = -1;
  int usage = -1;
  int report_count = 0, report_size = 0;

  bool found_any = false;

  int bit_offset = 0;

  while(p != desc_end) {
    uint8_t b = *p++;

    int len = b & 0x3;
    int type = (b >> 2) & 0x3;
    int tag = b >> 4;

    if(type == RI_TYPE_MAIN) {
      // ignore constants
      if(tag == RI_MAIN_INPUT) {
        if(usage_page == HID_USAGE_PAGE_DESKTOP && usage == HID_USAGE_DESKTOP_X) {
          stick_offset = bit_offset;
          hid_report_id = report_id; // assume everything is in the same report as the stick... and that the first x/y is the stick
          found_any = true;
        } else if(usage_page == HID_USAGE_PAGE_DESKTOP && usage == HID_USAGE_DESKTOP_HAT_SWITCH) {
          hat_offset = bit_offset;
          found_any = true;
        } else if(usage_page == HID_USAGE_PAGE_BUTTON && !(*p & HID_CONSTANT)) {
          // assume this is "the buttons"
          buttons_offset = bit_offset;
          num_buttons = report_count;
          found_any = true;
        }

        usage = -1;
        bit_offset += report_size * report_count;
      } else if(tag == RI_MAIN_COLLECTION) {
        usage = -1; // check that this is gamepad?
      }
    } else if(type == RI_TYPE_GLOBAL) {
      if(tag == RI_GLOBAL_USAGE_PAGE)
        usage_page = *p;
      else if(tag == RI_GLOBAL_REPORT_SIZE)
        report_size = *p;
      else if(tag == RI_GLOBAL_REPORT_ID) {
        report_id = *p;
        bit_offset = 0;
      } else if(tag == RI_GLOBAL_REPORT_COUNT)
        report_count = *p;


    } else if(type == RI_TYPE_LOCAL) {
      if(tag == RI_LOCAL_USAGE && usage == -1)
        usage = *p; // FIXME: multiple usages are a thing
    }

    p += len;
  }

  // don't bother requesting reports we can't parse
  if(!found_any)
    return;

  hid_gamepad_id = (vid << 16) | pid;

  if(!tuh_hid_receive_report(dev_addr, instance)) {
    printf("Cound not request report!\n");
  }
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  hid_keyboard_detected = false;
  hid_gamepad_id = 0;
}

// should this be here or in input.cpp?
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {

  auto report_data = hid_report_id == -1 ? report : report + 1;

  auto protocol = tuh_hid_interface_protocol(dev_addr, instance);

  if(protocol == HID_ITF_PROTOCOL_KEYBOARD) {
    hid_keyboard_detected = true;
    auto keyboard_report = (hid_keyboard_report_t const*) report;
    memcpy(hid_keys, keyboard_report->keycode, 6);

    tuh_hid_receive_report(dev_addr, instance);
    return;
  }

  // check report id if we have one
  if(hid_report_id == -1 || report[0] == hid_report_id) {
    // I hope these are reasonably aligned
    if(hat_offset != 0xFFFF)
      hid_hat = (report_data[hat_offset / 8] >> (hat_offset % 8)) & 0xF;
    else
      hid_hat = 8;

    hid_joystick[0] = report_data[stick_offset / 8];
    hid_joystick[1] = report_data[stick_offset / 8 + 1];

    // get up to 32 buttons
    hid_buttons = 0;
    int bits = buttons_offset % 8;
    int i = 0;
    auto p = report_data + buttons_offset / 8;

    // partial byte
    if(bits) {
      hid_buttons |= (*p++) >> bits;
      i += 8 - bits;
    }

    for(; i < num_buttons; i+= 8)
      hid_buttons |= (*p++) << i;
  }

  // next report
  tuh_hid_receive_report(dev_addr, instance);
}
#endif

// cdc
static uint8_t cdc_index = 0; // TODO: multiple devices?

void tuh_cdc_mount_cb(uint8_t idx) {
  cdc_index = idx;

  send_multiplayer_handshake();
}

void init_usb() {
  tusb_init();
}

void update_usb() {
  tuh_task();

  // TODO: resend multiplayer handshake
}

bool usb_cdc_connected() {
  return tuh_cdc_mounted(cdc_index);
}

uint16_t usb_cdc_read(uint8_t *data, uint16_t len) {
  return tuh_cdc_read(cdc_index, data, len);
}

uint32_t usb_cdc_read_available() {
  return tuh_cdc_read_available(cdc_index);
}

void usb_cdc_write(const uint8_t *data, uint16_t len) {
  uint32_t done = tuh_cdc_write(cdc_index,data, len);

  while(done < len) {
    tuh_task();
    if(!tuh_cdc_mounted(cdc_index))
      break;

    done += tuh_cdc_write(cdc_index, data + done, len - done);
  }
}

void usb_cdc_flush_write() {
  tuh_cdc_write_flush(cdc_index);
}

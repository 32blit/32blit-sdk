#include "usb.hpp"

#include "multiplayer.hpp"

#include "tusb.h"

#include "config.h"

// hid
#ifdef INPUT_USB_HID

static int hid_report_id = -1;
static uint16_t buttons_offset = 0, num_buttons = 0;
static uint16_t hat_offset = 0xFFFF, stick_offset = 0;
static uint8_t axis_size = 8;

enum class SwitchProInit : uint8_t {
  Mounted = 0,
  Handshake,
  ReadCalibration,
};

static SwitchProInit switch_pro_init_state = SwitchProInit::Mounted;
static uint8_t switch_pro_seq = 0;
static uint16_t switch_pro_calibration_x[2], switch_pro_calibration_y[2];

uint32_t hid_gamepad_id = 0;
bool hid_keyboard_detected = false;
uint8_t hid_joystick[2]{0x80, 0x80};
uint8_t hid_hat = 8;
uint32_t hid_buttons = 0;
uint8_t hid_keys[6]{};

static void switch_pro_read_flash(uint8_t dev_addr, uint8_t instance, uint32_t addr, uint8_t len) {
  uint8_t buf[16];
  buf[0] = 1; // rumble + command
  buf[1] = switch_pro_seq;
  // rumble data
  buf[2] = 0; buf[3] = 1; buf[4] = buf[5] = 0x40;
  buf[6] = 0; buf[7] = 1; buf[8] = buf[9] = 0x40;

  buf[10] = 0x10; // read flash
  buf[11] =  addr        & 0xFF;
  buf[12] = (addr >>  8) & 0xFF;
  buf[13] = (addr >> 16) & 0xFF;
  buf[14] = (addr >> 24) & 0xFF;
  buf[15] = len;

#if TUSB_VERSION_MINOR >= 16
  tuh_hid_send_report(dev_addr, instance, 0, buf, 16);
#endif

  switch_pro_seq++;
  if(switch_pro_seq > 0xF)
    switch_pro_seq = 0;
}

static void switch_pro_mount(uint8_t dev_addr, uint8_t instance) {
  uint8_t data = 2; // handshake
#if TUSB_VERSION_MINOR >= 16
  tuh_hid_send_report(dev_addr, instance, 0x80, &data, 1);
#endif

  switch_pro_init_state = SwitchProInit::Mounted;

  // report descriptor is inaccurate
  hid_report_id = 0x30;
  buttons_offset = 2 * 8;
  num_buttons = 24;
  stick_offset = 5 * 8;
  axis_size = 12;
  hat_offset = 0xFFFF; // no hat, only buttons
}

static void switch_pro_parse_left_stick_calibration(const uint8_t *data) {
  uint16_t max_x    = data[0]      | (data[1] & 0xF) << 8;
  uint16_t max_y    = data[1] >> 4 |  data[2]        << 4;
  uint16_t center_x = data[3]      | (data[4] & 0xF) << 8;
  uint16_t center_y = data[4] >> 4 |  data[5]        << 4;
  uint16_t min_x    = data[6]      | (data[7] & 0xF) << 8;
  uint16_t min_y    = data[7] >> 4 |  data[8]        << 4;

  switch_pro_calibration_x[0] = center_x - min_x;
  switch_pro_calibration_x[1] = center_x + max_x - switch_pro_calibration_x[0];
  switch_pro_calibration_y[0] = center_y - min_y;
  switch_pro_calibration_y[1] = center_y + max_y - switch_pro_calibration_y[0];
}

static void switch_pro_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {

  if(switch_pro_init_state == SwitchProInit::Handshake) {
    // first report after sending usb enable
    // request left stick calibration data
    switch_pro_read_flash(dev_addr, instance, 0x8010, 11);
    switch_pro_init_state = SwitchProInit::ReadCalibration;
  }

  if(report[0] == 0x81 && report[1] == 2) { // handshake
    uint8_t data = 4; // disable bluetooth / enable usb
#if TUSB_VERSION_MINOR >= 16
    tuh_hid_send_report(dev_addr, instance, 0x80, &data, 1);
#endif
    switch_pro_init_state = SwitchProInit::Handshake;
  } else if(report[0] == 0x21) {
    // response to cmd
    auto cmd = report[14];

    if(cmd == 0x10) { // flash read
      uint32_t addr = report[15] | report[16] << 8 | report[17] << 16 | report[18] << 24;
      // uint8_t len = report[19];
      auto read_data = report + 20;

      if(addr == 0x603D) {
        // factory left stick calibration
        switch_pro_parse_left_stick_calibration(read_data);
      } else if(addr == 0x8010) {
        // user left stick calibration
        if(read_data[0] == 0xB2 && read_data[1] == 0xA1) {
          // magic present, use it
          switch_pro_parse_left_stick_calibration(read_data + 2);
        } else {
          // request factory calibration
          switch_pro_read_flash(dev_addr, instance, 0x603D, 9);
        }
      }
    }
  }
}

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

  hat_offset = 0xFFFF;
  axis_size = 8;

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

  // switch pro controller
  if(hid_gamepad_id == 0x057E2009)
    switch_pro_mount(dev_addr, instance);

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

  // switch pro controller setup
  if(hid_gamepad_id == 0x057E2009)
    switch_pro_report(dev_addr, instance, report, len);

  // check report id if we have one
  if(hid_report_id == -1 || report[0] == hid_report_id) {
    // I hope these are reasonably aligned
    if(hat_offset != 0xFFFF)
      hid_hat = (report_data[hat_offset / 8] >> (hat_offset % 8)) & 0xF;
    else
      hid_hat = 8;

    if(axis_size == 8) {
      hid_joystick[0] = report_data[stick_offset / 8];
      hid_joystick[1] = report_data[stick_offset / 8 + 1];
    } else if(axis_size == 12) {
      uint16_t x = report_data[stick_offset / 8] | (report_data[stick_offset / 8 + 1] & 0xF) << 8;
      uint16_t y = report_data[stick_offset / 8 + 1] >> 4 | (report_data[stick_offset / 8 + 2]) << 4;

      // take the high bits
      hid_joystick[0] = x >> 4;
      hid_joystick[1] = y >> 4;

      // apply switch pro calibration
      if(hid_gamepad_id == 0x057E2009) {
        int calib_x = (x - switch_pro_calibration_x[0]) * 255 / switch_pro_calibration_x[1];
        int calib_y = (y - switch_pro_calibration_y[0]) * 255 / switch_pro_calibration_y[1];

        // clamp
        calib_x = calib_x < 0 ? 0 : (calib_x > 255 ? 255 : calib_x);
        calib_y = calib_y < 0 ? 0 : (calib_y > 255 ? 255 : calib_y);

        hid_joystick[0] = calib_x;
        hid_joystick[1] = 0xFF - calib_y;
      }
    }

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

void usb_debug(const char *message) {

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

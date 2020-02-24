#pragma once

#include <stdint.h>
#include <map>
#include "usbd_cdc.h"

extern "C" {
  extern USBD_CDC_ItfTypeDef USBD_Interface_fops_HS;
}

namespace usb_serial {

  enum class CommandState {STREAM, END, ERROR, TIMEOUT};

  typedef CommandState (*CommandHandler)(CommandState state, char *data, uint32_t length);

  struct Packet {
    char data[64];
    uint8_t length = 0;
  };

  extern std::map<std::string, CommandHandler> handlers;
  
  void init();
  void data_received(uint32_t length);
  bool transmit(const char *p);
  void parse_command();
  void register_command_handler(std::string command, CommandHandler handler);    

}

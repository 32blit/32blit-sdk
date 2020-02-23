#pragma once

#include <stdint.h>
#include <map>

namespace cdc {

  typedef bool (*CommandHandler)(char *data, uint32_t length);

  struct Packet {
    char data[64];
    uint8_t length = 0;
  };

  extern std::map<std::string, CommandHandler> handlers;
  
  void init();
  void data_received(uint32_t length);  
  void parse_command();
  void register_command_handler(std::string command, CommandHandler handler);    

}

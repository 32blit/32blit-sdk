#pragma once

#include <stdint.h>
#include <map>

namespace cdc {

  typedef bool (*CommandHandler)(char *data, uint32_t length);

  extern std::map<std::string, CommandHandler> handlers;

  void reset_rx_buffer();
  void data_received(uint32_t length);  
  void parse_command();
  void register_command_handler(std::string command, CommandHandler handler);    

}

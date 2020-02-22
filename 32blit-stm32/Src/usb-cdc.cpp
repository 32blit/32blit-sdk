#include "32blit.h"
#include "core-debug.hpp"
#include "usbd_cdc.h"
#include "usbd_core.h"
#include "usb-cdc.hpp"

#include <cstring>
#include <string>

extern USBD_HandleTypeDef hUsbDeviceHS;

namespace cdc {

  std::map<std::string, CommandHandler> handlers;

  struct {
    char data[64];
    uint32_t length = 0;
  } rx_buffer;

  // register a function to handle a new USB serial command
  void register_command_handler(std::string command, CommandHandler handler) {
    handlers[command] = handler;
  }

  void data_received(uint32_t length) {
    rx_buffer.length = length;
  }

  // stores the length of the current rx buffer and swaps to the other buffer if
  // it is free for use returning a pointer to the data member (or null if not free)
  void reset_rx_buffer() {      
    rx_buffer.length = 0;

    USBD_CDC_SetRxBuffer(&hUsbDeviceHS, rx_buffer.data);
    USBD_CDC_ReceivePacket(&hUsbDeviceHS);
  }

  // check for the incoming command identifier and matches it to the 
  // appropriate handler, then streams all incoming data directly to
  // the handler until the command processing is complete
  void parse_command() {
    static CommandHandler handler = nullptr;
    
    if(rx_buffer.length > 0) {   
     // debug::debug(std::to_string(rx_buffer.length).c_str());
      uint32_t s = blit::now();
      if(!handler) {                
        // if no handler assigned yet then we're waiting for the command
        // search through the handler list to see if we have a matching one
        for(auto c : handlers) {
          if(strcmp(c.first.c_str(), rx_buffer.data) == 0) {
            // found the command so assign the handler and call it with the
            // first packet of data
            handler = c.second;
            uint32_t command_length = strlen(rx_buffer.data) + 1;

            bool done = handler(rx_buffer.data + command_length, rx_buffer.length - command_length);

            if(done) {
              handler = nullptr;
            }
          }
        }
      } else {
        // the command handle must return true when it has finished processing
        // the entire command (even if this is across multiple packets)
        bool done = handler(rx_buffer.data, rx_buffer.length);

        if(done) {
          handler = nullptr;
        }
      }

      reset_rx_buffer();
    }    
  }
}
#include "32blit.h"
#include "core-debug.hpp"
#include "usbd_cdc.h"
#include "usbd_core.h"
#include "usb-cdc.hpp"

#include <cstring>
#include <string>
#include <stdint.h>
#include <queue>

extern USBD_HandleTypeDef hUsbDeviceHS;

namespace cdc {

  std::map<std::string, CommandHandler> handlers;

  // declare storage for packets buffer
  constexpr uint32_t MAX_PARSE_PACKETS = 16;
  Packet packets[MAX_PARSE_PACKETS];
  std::vector<Packet*> free_packets;

  Packet *rx_packet = nullptr;
  std::queue<Packet*> parse_queue;
  
  void request_new_packet();

  void init() {
    debug::debug("CDC INIT");
    
    // push the packet buffers into the free packets collection
    for(uint32_t i = 0; i < MAX_PARSE_PACKETS; i++) {
      free_packets.push_back(&packets[i]);
    }

    //request_new_packet();
  }
  
  void request_new_packet() {    
    // if we have space to receive more packets then return false
//    std::string m = "FREE ";
    //m += std::to_string(free_packets.size());
    //debug::debug(m);

    if(rx_packet == nullptr && free_packets.size() > 0) {
      rx_packet = free_packets.back();
      free_packets.pop_back();

      USBD_CDC_SetRxBuffer(&hUsbDeviceHS, rx_packet->data);
      USBD_CDC_ReceivePacket(&hUsbDeviceHS); 
    }
  }

  void data_received(uint32_t length) {      
    // take a copy of the newly received packet into the parse queue
    rx_packet->length = length;
    parse_queue.push(rx_packet);

    // request a new packet of data
    rx_packet = nullptr;
    request_new_packet();
  }

  // register a function to handle a new USB serial command
  void register_command_handler(std::string command, CommandHandler handler) {
    handlers[command] = handler;
  }

  // check for the incoming command identifier and matches it to the 
  // appropriate handler, then streams all incoming data directly to
  // the handler until the command processing is complete
  void parse_command() {
    static uint32_t parse_wait_ms = 1000;
    static uint32_t last_parse_time_ms = blit::now();
    static CommandHandler handler = nullptr;
    
    uint32_t time_ms = blit::now();
    if(time_ms - last_parse_time_ms < parse_wait_ms) {
      return;
    }

    last_parse_time_ms = time_ms;

    std::string m = "PARSE ";
    m += "QUEUE: ";
    m += std::to_string(parse_queue.size());
    m += " - FREE: ";
    m += std::to_string(free_packets.size());
    debug::debug(m);

    while(parse_queue.size() > 0) {
      // fetch the oldest packet from the queue
      Packet *packet = parse_queue.front();
      parse_queue.pop();
      
      if(!handler) {                
        // if no handler assigned yet then we're waiting for the command
        // search through the handler list to see if we have a matching one
        for(auto c : handlers) {
          if(strcmp(c.first.c_str(), packet->data) == 0) {
            // found the command so assign the handler and call it with the
            // first packet of data
            handler = c.second;
            uint32_t command_length = strlen(packet->data) + 1;

            bool done = handler(packet->data + command_length, packet->length - command_length);

            if(done) {
              handler = nullptr;
            }
          }
        }
      } else {
        // the command handle must return true when it has finished processing
        // the entire command (even if this is across multiple packets)
        bool done = handler(packet->data, packet->length);

        if(done) {
          handler = nullptr;
        }
      }  

      free_packets.push_back(packet);      
    }

    request_new_packet();
  }
}
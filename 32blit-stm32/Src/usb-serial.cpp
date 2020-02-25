/*
  usb serial command processor. streams in data from usb cdc 
  serial and dispatches it to the appropriate handler.

  this work is based on heavily on the original concept implemented
  by andrewcapon. any genius may safely be attributed to him while 
  any faults can be attributed to lowfatcode.

  author: lowfatcode (standing on the shoulders of andrewcapon)
  created: 23rd feb 2020
*/

#include "32blit.h"
#include "core-debug.hpp"
#include "usbd_cdc.h"
#include "usbd_core.h"
#include "usb-serial.hpp"

#include <cstring>
#include <string>
#include <stdint.h>
#include <queue>

extern USBD_HandleTypeDef hUsbDeviceHS;

static int8_t CDC_Init_HS(void)
{
  usb_serial::init();  

  return USBD_OK;
}

static int8_t CDC_DeInit_HS()
{
  return USBD_OK;
}

static int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  // 115200bps, 1stop, no parity, 8bit
  static uint8_t lineCoding[7] = { 0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08 };

  switch(cmd) {
  case CDC_SEND_ENCAPSULATED_COMMAND: break;
  case CDC_GET_ENCAPSULATED_RESPONSE: break;
  case CDC_SET_COMM_FEATURE: break;
  case CDC_GET_COMM_FEATURE: break;
  case CDC_CLEAR_COMM_FEATURE: break;
  case CDC_SET_CONTROL_LINE_STATE: break;
  case CDC_SEND_BREAK: break;
  case CDC_SET_LINE_CODING:
    memcpy(lineCoding, pbuf, sizeof(lineCoding)); 
    break;
  case CDC_GET_LINE_CODING: 
    memcpy(pbuf, lineCoding, sizeof(lineCoding)); 
    break;
  default:
    break;
  }

  return (USBD_OK);
}

static int8_t CDC_Receive_HS(uint8_t* Buf, uint32_t *Len)
{ 
  usb_serial::data_received(*Len);
  return USBD_OK;
}

uint8_t CDC_Transmit_HS(uint8_t* Buf, uint16_t Len)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
  
  if(hcdc->TxState != 0) { return USBD_BUSY; }

  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, Buf, Len);  
  return USBD_CDC_TransmitPacket(&hUsbDeviceHS);
}

USBD_CDC_ItfTypeDef USBD_Interface_fops_HS =
{
  CDC_Init_HS, CDC_DeInit_HS, CDC_Control_HS, CDC_Receive_HS
};


namespace usb_serial {

  std::map<std::string, CommandHandler> handlers;

  constexpr uint32_t MAX_PACKETS = 64;
  Packet packets[MAX_PACKETS];
  std::vector<Packet*> free_packets;

  Packet *rx_packet = nullptr;
  std::queue<Packet*> parse_queue;
  
  void request_new_packet();

  void init() {
    // push the packet buffers into the free packets collection
    for(uint32_t i = 0; i < MAX_PACKETS; i++) {
      free_packets.push_back(&packets[i]);
    }

    request_new_packet();
  }

  bool transmit(const char *p) {
    return CDC_Transmit_HS(p, strlen(p) + 1) == USBD_OK;
  }
  
  void request_new_packet() {    
    // if we have space to receive more packets then return false
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
    constexpr uint32_t PARSE_STREAM_TIMEOUT = 500;
    static    uint32_t last_packet_time_ms = blit::now();

    static CommandHandler handler = nullptr;
        
    request_new_packet();
    
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

            // trim the command from this packet and adjust the length allowing
            // the rest of the packet to be processed by the handler
            uint32_t command_length = strlen(packet->data) + 1;            
            packet->length -= command_length;
            memcpy(packet->data, packet->data + command_length, packet->length);            
          }
        }
      } 
      
      if(handler) {
        if(packet->length > 0) {
          // the command handle must return true when it has finished processing
          // the entire command (even if this is across multiple packets)
          CommandState result = handler(CommandState::STREAM, packet->data, packet->length);
          
          if(result == CommandState::END) {    
            // command stream is complete, detach handler
            handler = nullptr;
          }

          if(result == CommandState::ERROR) {          
            // TODO: show error message
            handler = nullptr;
          }
        }  
      }

      free_packets.push_back(packet);   
      last_packet_time_ms = blit::now();               
    }        

    // check that the time since the last packet we received hasn't exceeded
    // the timeout. if it has then notify the current handler so that it can
    // tidy up and prepare to receive any future commands.
    uint32_t ms_since_last_packet = blit::now() - last_packet_time_ms;
    if(handler && (ms_since_last_packet > PARSE_STREAM_TIMEOUT)) {
      CommandState result = handler(CommandState::TIMEOUT, nullptr, 0);
      handler = nullptr;
    }
  }
}

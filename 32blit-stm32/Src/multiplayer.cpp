#include "usbd_def.h"
#include "usbd_cdc_if.h"
#include "usb_host.h"
#include "usbh_cdc.h"

#include "CDCCommandHandler.h"
#include "CDCCommandStream.h"

#include "engine/api_private.hpp"

extern CDCCommandStream g_commandStream;
extern USBD_HandleTypeDef hUsbDeviceHS;
extern USBH_HandleTypeDef hUsbHostHS;

using namespace blit;

namespace multiplayer {
  class CDCUserHandler : public CDCCommandHandler
  {
  public:
    StreamResult StreamData(CDCDataStream &dataStream) override {
  
      // get length and allocate buf
      if(!buf) {
        if(!dataStream.Get(length))
          return srNeedData;

        delete[] buf; // just in case, prevent leak
        buf = new uint8_t[length];
      }

      // read
      uint8_t byte;
      while(read < length && dataStream.Get(byte))
        buf[read++] = byte;

      // done, send to user
      if(read == length) {
        if(api.message_received)
          api.message_received(buf, length);

        delete[] buf;
        buf = nullptr;
        return srFinish;
      }

      return srContinue;
    }

    bool StreamInit(CDCFourCC uCommand) override {
      length = read = 0;
      return true;
    }

  private:
    uint16_t length = 0, read = 0;

    uint8_t *buf = nullptr;
  };

  CDCUserHandler cdc_user_handler;

  void init() {
    g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'U', 'S', 'E', 'R'>::value, &cdc_user_handler);
  }

  void cdc_send(const uint8_t *data, uint16_t length) {
    if(USB_GetMode(USB_OTG_HS)) { // host
      if(hUsbHostHS.gState != HOST_CLASS)
        return;
  
      // FIXME: should this be using the transmit callback and have a queue somewhere?
      USBH_CDC_Transmit(&hUsbHostHS, (uint8_t *)data, length);

      while(((CDC_HandleTypeDef *) hUsbHostHS.pActiveClass->pData)->data_tx_state != CDC_IDLE){
        MX_USB_HOST_Process();
        //HOST_CLASS
        if(hUsbHostHS.gState != HOST_CLASS)
          break; // stopped being connected
      }
    } else { // device
      while(hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED && CDC_Transmit_HS((uint8_t *)data, length) == USBD_BUSY);
    }
  }

  void send_message(const uint8_t *data, uint16_t length) {
    // header
    cdc_send((uint8_t *)"32BLUSER", 8);
    cdc_send((uint8_t *)&length, 2);

    cdc_send((uint8_t *)data, length);
  }
}
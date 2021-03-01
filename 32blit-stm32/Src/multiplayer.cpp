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
  bool enabled = false;
  bool peer_connected = false;

  void send_handshake();

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
        if(api.message_received && enabled)
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

  // this is also a disconnect/disable
  class CDCHandshakeHandler : public CDCCommandHandler {
  public:
    StreamResult StreamData(CDCDataStream &dataStream) override {

      uint8_t val;
      dataStream.Get(val);

      peer_connected = val != 0;

      // reply if we're not the host
      if(peer_connected && !USB_GetMode(USB_OTG_HS))
        send_handshake();

      return srFinish;
    }

    bool StreamInit(CDCFourCC uCommand) override {
      return true;
    }
  };

  CDCUserHandler cdc_user_handler;
  CDCHandshakeHandler cdc_handshake_handler;

  static uint32_t last_handshake_attempt = 0;

  void init() {
    g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'U', 'S', 'E', 'R'>::value, &cdc_user_handler);
    g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'M', 'L', 'T', 'I'>::value, &cdc_handshake_handler);
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

  void send_handshake() {
    uint8_t buf[]{'3', '2', 'B', 'L', 'M', 'L', 'T','I', enabled ? 1 : 0};
    cdc_send(buf, 9);
  }

  bool is_connected() {
    return enabled && peer_connected;
  }

  bool is_usb_connected() {
    if(USB_GetMode(USB_OTG_HS))
      return hUsbHostHS.gState == HOST_CLASS;
    else
      return hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED;
  }

  void set_enabled(bool enabled) {

    multiplayer::enabled = enabled;

    // disconnect
    if(!enabled)
      send_handshake();

    // if in device mode, disable/enable VBUS sensing
    // TODO: this will fail if we switch to host and back
    if(!USB_GetMode(USB_OTG_HS)) {
      uint32_t USBx_BASE = (uint32_t)USB_OTG_HS; // this is for USBx_DEVICE...
      USBx_DEVICE->DCTL |= USB_OTG_DCTL_SDIS;

      if(enabled) {
        USB_OTG_HS->GINTMSK &= ~(USB_OTG_GINTMSK_SRQIM | USB_OTG_GINTMSK_OTGINT);

        USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

        USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
        USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
      } else {
        USB_OTG_HS->GOTGCTL &= ~USB_OTG_GOTGCTL_BVALOEN;
        USB_OTG_HS->GOTGCTL &= ~USB_OTG_GOTGCTL_BVALOVAL;

        USB_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBDEN;

        USB_OTG_HS->GINTMSK |= USB_OTG_GINTMSK_SRQIM | USB_OTG_GINTMSK_OTGINT;
      }

      USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_SDIS;
    }
  }

  void send_message(const uint8_t *data, uint16_t length) {
    if(!peer_connected)
      return;

    // header
    cdc_send((uint8_t *)"32BLUSER", 8);
    cdc_send((uint8_t *)&length, 2);

    cdc_send((uint8_t *)data, length);
  }

  void update() {
    // reset peer_connected if the USB was disconnected
    if(peer_connected && !is_usb_connected())
      peer_connected = false;

    // attempt handshake if we're the host
    if(!peer_connected && USB_GetMode(USB_OTG_HS) && HAL_GetTick() - last_handshake_attempt > 1000) {
      send_handshake();
      last_handshake_attempt = HAL_GetTick();
    }
  }
}

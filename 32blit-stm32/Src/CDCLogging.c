/*
 * CDCLogging.c
 *
 *  Created on: 4 Jan 2020
 *      Author: andrewcapon
 */

#include "usbd_def.h"
#include "usbd_cdc_if.h"
#include "USBManager.h"

extern USBD_HandleTypeDef hUsbDeviceHS;

bool g_bConsumerConnected = true;
extern USBManager g_usbManager;

extern "C" // C linkage because we are compiling c files with a c++ compiler
{
	int _write(int file, char *ptr, int len)
	{
    // disable in MassMedia mode
    if(g_usbManager.GetType() == USBManager::usbtCDC)
    {
      // The mad STM CDC implementation relies on a USB packet being received to set TxState
      // Also calls to CDC_Transmit_HS do not buffer the data so we have to rely on TxState before sending new data.
      // So if there is no consumer running at the other end we will hang, so we need to check for this
      if(g_bConsumerConnected)
      {
        uint32_t tickstart = HAL_GetTick();
        while(g_bConsumerConnected && CDC_Transmit_HS((uint8_t *)ptr, len) == USBD_BUSY)
          g_bConsumerConnected = !(HAL_GetTick() > (tickstart + 2));
      }
      else
      {
        USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
        g_bConsumerConnected = !(hcdc->TxState != 0);
      }
    }
		return len;
	}
}


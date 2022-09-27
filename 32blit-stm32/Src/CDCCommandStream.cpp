/*
 * CDCCommandStream.cpp
 *
 *  Created on: 17 Jan 2020
 *      Author: andrewcapon
 */
#include "32blit.h"
#include "usbd_cdc_if.h"
#include "usbh_cdc.h"

extern USBD_HandleTypeDef hUsbDeviceHS;
extern USBH_HandleTypeDef hUsbHostHS;

#include "CDCCommandStream.h"

void CDCCommandStream::Init(void)
{
	m_state = stDetect;
	m_uHeaderScanPos = 0;
	m_bNeedsUSBResume = false;
	uLastResumeTime = HAL_GetTick();
}

void CDCCommandStream::AddCommandHandler(CDCCommandHandler::CDCFourCC uCommand, CDCCommandHandler *pCommandHandler)
{
	m_commandHandlers.emplace(uCommand, pCommandHandler);
}


void CDCCommandStream::LogTimeTaken(CDCCommandHandler::StreamResult result, uint32_t uBytesHandled)
{
	// can be used to log time taken
}


void CDCCommandStream::Stream(void)
{
  if(m_state == stDisabled)
    return;

  if(m_bNeedsUSBResume) // FIFO Full, so empty and resume USB
  {
    while(CDCFifoElement *pElement = GetFifoReadElement())
    {
      if(pElement->m_uLen)
        Stream(pElement->m_data, pElement->m_uLen);
      ReleaseFifoReadElement();
    }

    Resume();
  }
  else
  {
    if(HAL_GetTick() > uLastResumeTime + 10) // USB Stalled, empty FIFO
    {
      // empty fifo
      while(CDCFifoElement *pElement = GetFifoReadElement())
      {
        if(pElement->m_uLen)
          Stream(pElement->m_data, pElement->m_uLen);
        ReleaseFifoReadElement();
      }
    }
  }
}

uint8_t CDCCommandStream::Stream(uint8_t *data, uint32_t len)
{
  static const uint8_t header[4] = { '3', '2', 'B', 'L' };

	uint8_t   *pScanPos  = data;

  while(pScanPos < (data+len))
  {
    if(m_state == stDetect)
    {
      bool bHeaderFound = false;
      while ((!bHeaderFound) && (pScanPos < (data+len)))
      {
        if(*pScanPos == header[m_uHeaderScanPos])
        {
          if(m_uHeaderScanPos == 3)
          {
            // header found
            bHeaderFound = true;
            m_uHeaderScanPos = 0;
            m_uCommandScanPos = 0;
          }
          else
            m_uHeaderScanPos++;
        }
        else
        {
          m_uHeaderScanPos = 0;
          m_uCommandScanPos = 0;
        }
        pScanPos++;
      }

      if(bHeaderFound)
      {
        // next command byte could be in next call
        while((m_state == stDetect) && (m_uCommandScanPos < 4))
        {
          uint8_t  *pCommandByte = (uint8_t *)(&uCommand)+m_uCommandScanPos;
          if(pScanPos >= (data + len)) // rest in next packet
            m_state = stDetectCommandWord;
          else
          {
            *pCommandByte = *pScanPos++;
            if(m_uCommandScanPos == 3)
            {
              m_state = stDispatch;
            }
            m_uCommandScanPos++;
          }
        }
      }
    }
    else
    {
      if(m_state == stDetectCommandWord)
      {
        while(m_uCommandScanPos < 4)
        {
          uint8_t *pCommandByte = (uint8_t *)(&uCommand)+m_uCommandScanPos;
          *pCommandByte = *pScanPos++;
          m_uCommandScanPos++;
        }
        m_state = stDispatch;
      }
    }

    if(m_state == stDispatch)
    {
      m_uRetryCount = 0;
      m_uDispatchTime = HAL_GetTick();

      auto handler = m_commandHandlers.find(uCommand);
      m_pCurrentCommandHandler = handler == m_commandHandlers.end() ? nullptr : handler->second;

      if(m_pCurrentCommandHandler)
      {
        if(m_pCurrentCommandHandler->StreamInit(uCommand)) {
          m_state = stProcessing;
          m_dataStream.Clear(); // make sure there isn't any leftover data in the buffer
        } else
        {
          m_state = stDetect;
          LogTimeTaken(CDCCommandHandler::srFinish, 0);
        }
      }
      else
        m_state = stDetect; // No handler go back to detect state
    }

    if(m_state == stProcessing)
    {
      auto added_len = len - (pScanPos - data);

      m_dataStream.AddData(pScanPos, added_len);
      CDCCommandHandler::StreamResult result = m_pCurrentCommandHandler->StreamData(m_dataStream);

      switch(result)
      {
        case CDCCommandHandler::srError:
        case CDCCommandHandler::srFinish:
        {
          // handler has finished or failed go back to detect state
          m_state = stDetect;
          LogTimeTaken(result, m_pCurrentCommandHandler->GetBytesHandled());
        }
        break;

        case CDCCommandHandler::srNeedData:
          m_uRetryCount++;
          if(m_uRetryCount > 1)
          {
            m_state = stDetect;
            LogTimeTaken(result, m_pCurrentCommandHandler->GetBytesHandled());
          }
        return false;

        case CDCCommandHandler::srContinue:
        break;
      }

      // handled all data or need more
      if(m_dataStream.GetStreamLength() == 0 || m_state != stDetect)
        break;
      else // some data left, advance scan pos and try again
        pScanPos += added_len - std::min(added_len, m_dataStream.GetStreamLength());
    }

  }

	return m_state != stDetect;
}

void CDCCommandStream::SetParsingEnabled(bool enabled)
{
  if(enabled && m_state == stDisabled)
    m_state = stDetect;
  else if(!enabled)
    m_state = stDisabled;
}

uint32_t CDCCommandStream::Read(uint8_t *data, uint32_t len)
{
  if(m_state != stDisabled)
    return 0; // can't manually read if we're streaming

  uint32_t off = 0;
  auto elOff = m_uCurElementOff;
  bool releasedElement = false;

  while(CDCFifoElement *element = GetFifoReadElement())
  {
    auto count = std::min(len - off, uint32_t(element->m_uLen - elOff));

    memcpy(data + off, element->m_data + elOff, count);
    off += count;

    if(elOff + count == element->m_uLen)
    {
      // read entire element
      ReleaseFifoReadElement();
      releasedElement = true;
      elOff = 0;
    }
    else
    {
      elOff += count;
      break;
    }
  }

  m_uCurElementOff = elOff;

  // resume
  if(m_bNeedsUSBResume && releasedElement)
    Resume();

  return off;
}

uint32_t CDCCommandStream::GetTimeTaken(void)
{
	return HAL_GetTick() - m_uDispatchTime;
}


uint8_t	*CDCCommandStream::GetFifoWriteBuffer(void)
{
	uint8_t *pData = NULL;
	if((m_uFifoWritePos + 1) % CDC_FIFO_BUFFERS != m_uFifoReadPos)
	{
		pData = m_fifoElements[m_uFifoWritePos].m_data;
	}
	else
		m_bNeedsUSBResume = true;

	return pData;
}

void	CDCCommandStream::ReleaseFifoWriteBuffer(uint8_t uLen)
{
	m_fifoElements[m_uFifoWritePos].m_uLen = uLen;
	m_uFifoWritePos++;
	if(m_uFifoWritePos == CDC_FIFO_BUFFERS)
		m_uFifoWritePos = 0;
}

CDCFifoElement  *CDCCommandStream::GetFifoReadElement(void)
{
	CDCFifoElement *pElement = NULL;
	if(m_uFifoReadPos != m_uFifoWritePos)
		pElement = &m_fifoElements[m_uFifoReadPos];
	return pElement;
}

void CDCCommandStream::ReleaseFifoReadElement(void)
{
	m_uFifoReadPos++;
	if(m_uFifoReadPos == CDC_FIFO_BUFFERS)
		m_uFifoReadPos = 0;
}

void CDCCommandStream::Resume()
{
  if(USB_GetMode(USB_OTG_HS))
  {
    USBH_CDC_Receive(&hUsbHostHS, GetFifoWriteBuffer(), 64);
  }
  else
  {
    USBD_CDC_SetRxBuffer(&hUsbDeviceHS, GetFifoWriteBuffer());
    USBD_CDC_ReceivePacket(&hUsbDeviceHS);
  }
  m_bNeedsUSBResume = false;
  uLastResumeTime = HAL_GetTick();
}

// usb host glue
extern CDCCommandStream g_commandStream;

void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
{
  	g_commandStream.ReleaseFifoWriteBuffer(USBH_CDC_GetLastReceivedDataSize(phost));

	// If a new writebuffer is available, set RxBuffer and requext next USB packet
	if(uint8_t *pBuffer = g_commandStream.GetFifoWriteBuffer())
    	USBH_CDC_Receive(phost, pBuffer, 64);
}

extern "C" void usb_host_ready(USBH_HandleTypeDef *phost) {
	if(uint8_t *pBuffer = g_commandStream.GetFifoWriteBuffer())
    	USBH_CDC_Receive(phost, pBuffer, 64);
}

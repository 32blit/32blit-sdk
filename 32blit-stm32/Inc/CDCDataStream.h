/*
 * CDCDataStream.h
 *
 *  Created on: 17 Jan 2020
 *      Author: andrewcapon
 */

#ifndef SRC_CDCDATASTREAM_H_
#define SRC_CDCDATASTREAM_H_

#include "stm32h7xx_hal.h"
#include <cstring>

class CDCDataStream
{
public:
	CDCDataStream() : m_pData(NULL), m_uLen(0)
	{

	}

	bool AddData(uint8_t *pData, uint32_t uLen)
	{
		if(m_uLen)
			memmove(buf, m_pData, m_uLen);

		memcpy(buf + m_uLen, pData, uLen);
		m_pData = buf;
		m_uLen += uLen;

		return true;
	}

  void Clear()
  {
    m_uLen = 0;
  }

	uint32_t GetStreamLength(void)
	{
		return m_uLen;
	}

	bool GetDataOfLength(void *pData, uint8_t uLen)
	{
		bool bResult = false;
		uint8_t *pByteData = (uint8_t *)pData;

		if(m_uLen >= uLen)
		{
			while(uLen--)
			{
				bResult = true;
				*pByteData++ = *m_pData++;
				m_uLen--;
			}
		}

		return bResult;
	}

	bool Get(uint32_t &uValue)
	{
		return GetDataOfLength(&uValue, 4);
	}

	bool Get(uint16_t &uValue)
	{
		return GetDataOfLength(&uValue, 2);
	}

	bool Get(uint8_t &uValue)
	{
		return GetDataOfLength(&uValue, 1);
	}

private:
	uint8_t		*m_pData;
	uint32_t 	m_uLen;

	uint8_t buf[64 + 4]; // slightly larger than a FIFO element
};


#endif /* SRC_CDCDATASTREAM_H_ */

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
#include <streambuf>
#include <istream>

class CDCMemBuffer : public std::basic_streambuf<char>
{
public:
	CDCMemBuffer(const uint8_t *pData, size_t uLen)
	{
    setg((char*)pData, (char*)pData, (char*)pData + uLen);
  }

	void SetData(const uint8_t *pData, size_t uLen)
	{
    setg((char*)pData, (char*)pData, (char*)pData + uLen);
  }
};

class CDCMemoryStream : public std::istream
{
public:
	CDCMemoryStream(const uint8_t *pData = NULL , size_t uLen = 0) : std::istream(&m_buffer), m_buffer(pData, uLen)
	{
    rdbuf(&m_buffer);
  }

	void SetData(const uint8_t *pData, size_t uLen)
	{
		clear();
		m_buffer.SetData(pData, uLen);
    rdbuf(&m_buffer);
	}
private:
	CDCMemBuffer m_buffer;
};


class CDCDataStream
{
public:
	CDCDataStream() : m_pData(NULL), m_uLen(0)
	{

	}

	bool AddData(uint8_t *pData, uint32_t uLen)
	{
		m_pData = pData;
		m_uLen  = uLen;

		return true;

	}

	uint32_t GetStreamLength(void)
	{
		return m_uLen;
	}


	bool GetMemoryStream(CDCMemoryStream &ms)
	{
		bool bResult = false;

		uint8_t *pData;
		uint32_t uLen;
		if(uLen)
		{
			ms.SetData(pData, uLen);
			bResult = true;
		}

		return bResult;
	}


	bool GetStreamData(uint8_t **ppData, uint32_t *puLen)
	{
		bool bResult = true;

		if(m_uLen)
		{
			*ppData = m_pData;
			*puLen = m_uLen;
			m_uLen = 0;
		}
		else
		{
			bResult = false;
		}

		return bResult;
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

	bool Get(uint8_t &uValue)
	{
		return GetDataOfLength(&uValue, 1);
	}

private:
	uint8_t		*m_pData;
	uint32_t 	m_uLen;
};


#endif /* SRC_CDCDATASTREAM_H_ */

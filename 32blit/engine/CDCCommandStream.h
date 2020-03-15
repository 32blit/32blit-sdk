/*
 * CDCCommandStream.h
 *
 *  Created on: 17 Jan 2020
 *      Author: andrewcapon
 */

#ifndef SRC_CDCCOMMANDSTREAM_H_
#define SRC_CDCCOMMANDSTREAM_H_

#include <map>

#include "engine/CDCCommandHandler.h"

// for receiving at fullspeed CDC we need as many fifo elements as the time taken in ms in the main loop
// Also for code that does not require fast streaming this could be set to 1
// So this will need tuning and define should come from cmake.
// If nothing from cmake default to 4 buffers

#ifndef CDC_FIFO_BUFFERS
#define CDC_FIFO_BUFFERS 4
#endif

struct CDCFifoElement
{
	uint8_t  m_data[64];
	uint8_t  m_uLen = 0;
};


class CDCCommandStream
{
public:
	CDCCommandStream() : m_uFifoReadPos(0), m_uFifoWritePos(0), m_uFifoUsedCount(0)
	{
		Init();
	}

	void Init(void);
	void AddCommandHandler(CDCCommandHandler::CDCFourCC uCommand, CDCCommandHandler *pCommandHandler);


	void     Stream(void);
	uint8_t  Stream(uint8_t *data, uint32_t len);

	// USB Packet fifo
	uint8_t					*GetFifoWriteBuffer(void);
	void						ReleaseFifoWriteBuffer(uint8_t uLen);
	CDCFifoElement  *GetFifoReadElement(void);
	void 						ReleaseFifoReadElement(void);
	bool						IsFifoFull(void)
	{
		return m_uFifoUsedCount==CDC_FIFO_BUFFERS;
	}


private:
	typedef enum { stDetect, stDetectCommandWord, stDispatch, stProcessing, stError } StreamState;

	StreamState m_state;

	uint8_t			m_header[4] = { '3', '2', 'B', 'L' };
	uint8_t			m_uHeaderScanPos = 0;
	uint8_t			m_uCommandScanPos = 0;


	CDCCommandHandler														*m_pCurrentCommandHandler;
	std::map<uint32_t, CDCCommandHandler*> 	m_commandHandlers;

	CDCDataStream	m_dataStream;

	uint8_t m_uRetryCount = 0;


	CDCFifoElement m_fifoElements[CDC_FIFO_BUFFERS];
	uint8_t				 m_uFifoReadPos;
	uint8_t				 m_uFifoWritePos;
	uint8_t				 m_uFifoUsedCount;

	bool					m_bNeedsUSBResume;



};

#endif /* SRC_CDCCOMMANDSTREAM_H_ */

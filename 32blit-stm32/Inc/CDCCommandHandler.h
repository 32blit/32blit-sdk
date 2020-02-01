/*
 * CDCCommandHandler.h
 *
 *  Created on: 17 Jan 2020
 *      Author: andrewcapon
 */

#ifndef SRC_CDCCOMMANDHANDLER_H_
#define SRC_CDCCOMMANDHANDLER_H_


#include "CDCDataStream.h"

class CDCCommandHandler
{
public:
	template <int a, int b, int c, int d>
	struct CDCFourCCMake
	{
	    static const unsigned int value = (((((d << 8) | c) << 8) | b) << 8) | a;
	};

	typedef uint32_t CDCFourCC;

	typedef enum { srError, srContinue, srFinish, srNeedData } StreamResult;

	static constexpr char *StreamResultStrings[4] = { (char *)"Error:", (char *)"Continue:", (char *)"Finish:", (char *)"NeedData:" };

	static char *GetStreamResultString(StreamResult result)
	{
		return CDCCommandHandler::StreamResultStrings[result];
	}

	virtual StreamResult 	StreamData(CDCDataStream &dataStream) = 0;
	virtual bool 	 				StreamInit(CDCFourCC uCommand) = 0;

	uint32_t GetBytesHandled(void)
	{
		return m_uBytesHandled;
	}

protected:
	uint32_t m_uBytesHandled = 0;
};

#endif /* SRC_CDCCOMMANDHANDLER_H_ */

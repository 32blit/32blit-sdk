/*
 * CDCInfoHandler.h
 *
 *  Created on: 18 Jan 2020
 *      Author: andrewcapon
 */

#ifndef SRC_CDCINFOHANDLER_H_
#define SRC_CDCINFOHANDLER_H_

#include "CDCCommandHandler.h"

class CDCInfoHandler : public CDCCommandHandler
{
public:
	virtual StreamResult StreamData(CDCDataStream &dataStream);
	virtual bool StreamInit(CDCFourCC uCommand);
};

#endif /* SRC_CDCRESETHANDLER_H_ */

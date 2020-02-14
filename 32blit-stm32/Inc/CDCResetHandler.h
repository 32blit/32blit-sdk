/*
 * CDCResetHandler.h
 *
 *  Created on: 18 Jan 2020
 *      Author: andrewcapon
 */

#ifndef SRC_CDCRESETHANDLER_H_
#define SRC_CDCRESETHANDLER_H_

#include "CDCCommandHandler.h"

class CDCResetHandler : public CDCCommandHandler
{
public:
	virtual StreamResult StreamData(CDCDataStream &dataStream);
	virtual bool StreamInit(CDCFourCC uCommand);
};

#endif /* SRC_CDCRESETHANDLER_H_ */

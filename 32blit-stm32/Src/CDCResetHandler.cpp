/*
 * CDCResetHandler.cpp
 *
 *  Created on: 18 Jan 2020
 *      Author: andrewcapon
 */

#include "CDCResetHandler.h"

bool CDCResetHandler::StreamInit(CDCFourCC uCommand)
{
	NVIC_SystemReset();

	return false;
}


CDCCommandHandler::StreamResult CDCResetHandler::StreamData(CDCDataStream &dataStream)
{
	return srFinish;
}

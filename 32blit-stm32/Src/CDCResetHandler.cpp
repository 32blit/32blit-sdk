/*
 * CDCResetHandler.cpp
 *
 *  Created on: 18 Jan 2020
 *      Author: andrewcapon
 */

#include "CDCResetHandler.h"
#include "32blit.h"

bool CDCResetHandler::StreamInit(CDCFourCC uCommand)
{
  if(uCommand == CDCCommandHandler::CDCFourCCMake<'S', 'W', 'I', 'T'>::value || blit_user_code_running())
    blit_switch_execution(0, false);
  else
	  NVIC_SystemReset();

	return false;
}


CDCCommandHandler::StreamResult CDCResetHandler::StreamData(CDCDataStream &dataStream)
{
	return srFinish;
}

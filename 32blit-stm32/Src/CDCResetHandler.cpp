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
#if EXTERNAL_LOAD_ADDRESS == 0x90000000
  if(uCommand == CDCCommandHandler::CDCFourCCMake<'S', 'W', 'I', 'T'>::value)
    blit_switch_execution(0); 
  else
	  NVIC_SystemReset();
#else
  blit_switch_execution(0); 
#endif  

	return false;
}


CDCCommandHandler::StreamResult CDCResetHandler::StreamData(CDCDataStream &dataStream)
{
	return srFinish;
}

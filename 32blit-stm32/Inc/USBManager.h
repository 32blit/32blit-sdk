#pragma once

#include "32blit.h"

class USBManager
{
public:
	typedef enum {usbtCDC, usbtMSC} Type;
	typedef enum {usbsCDC, usbsMSCInititalising, usbsMSCMounting, usbsMSCMounted, usbsMSCUnmounting, usbsMSCUnmounted} State;

	USBManager() : m_type(usbtMSC), m_state(usbsMSCInititalising), m_mountStartTime(0), m_unmountStartTime(0)
	{

	};

	void SetType(Type type)
	{
		m_type = type;
	}

	Type GetType(void)
	{
		return m_type;
	}

	void SetState(State state)
	{
		m_state = state;

		switch(m_state)
		{
			case usbsMSCMounting :
				m_mountStartTime = HAL_GetTick();
			break;

			case usbsMSCUnmounting :
				m_unmountStartTime = HAL_GetTick();
			break;

			default:
			break;
		}
	}

	State GetState(void)
	{
		// On linux we seem to get many start/stops so use a timer to set mount status.
		const uint32_t uMountUnmountTime = 1000;

		switch(m_state)
		{
			case usbsMSCMounting :
				if(HAL_GetTick() > m_mountStartTime  + uMountUnmountTime)
					m_state = usbsMSCMounted;
			break;

			case usbsMSCUnmounting :
				if(HAL_GetTick() > m_unmountStartTime  + uMountUnmountTime)
					m_state = usbsMSCUnmounted;
			break;

			default:
			break;
		}

		return m_state;
	}

	bool IsMSCReady(void)
	{
		 return m_state == usbsMSCInititalising || m_state == usbsMSCMounting || m_state == usbsMSCMounted;
	}

private:
	Type			m_type;
	State			m_state;
	uint32_t	m_mountStartTime;
	uint32_t	m_unmountStartTime;
};



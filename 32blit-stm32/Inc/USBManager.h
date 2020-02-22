#pragma once

#include "32blit.h"
#include "usb_device.h"
#include "i2c.h"

class USBManager
{
public:
	typedef enum {usbtCDC, usbtMSC} Type;
	typedef enum {usbsCDC, usbsMSCInititalising, usbsMSCMounting, usbsMSCMounted, usbsMSCUnmounting, usbsMSCUnmounted} State;

	USBManager() : m_type(usbtCDC), m_state(usbsCDC), m_mountStartTime(0), m_unmountStartTime(0), m_bHasActivity(false), m_bHasHadSomeActivity(false)
	{

	};

	void SetType(Type type)
	{
		if(m_type != type)
		{
			// tear down USB
			MX_USB_DEVICE_Deinit();
			m_type = type;

			// set start state
			if(m_type == usbtCDC)
				m_state = usbsCDC;
			else
			{
				m_bHasActivity = false;
				m_bHasHadSomeActivity = false;
				m_state = usbsMSCInititalising;
			}

			// recreate usb device
			MX_USB_DEVICE_Init();

      // re-init i2c
      MX_I2C4_Init();


		}
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
		const uint32_t uMountUnmountTime = 500;

		switch(m_state)
		{
			case usbsMSCInititalising:
				if(m_bHasHadSomeActivity)
					m_state = usbsMSCMounting; // for windows as we don;t ever get a scsi start message
			break;

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

	const char *GetStateName(void)
	{
		return m_stateNames[GetState()];
	}

	bool IsMSCReady(void)
	{
		 return m_state == usbsMSCInititalising || m_state == usbsMSCMounting || m_state == usbsMSCMounted;
	}

	bool HasHadActivity(void)
	{
		bool bHasActivity = m_bHasActivity;
		m_bHasActivity = false;
		return bHasActivity;
	}

	void LogActivity(void)
	{
		m_bHasActivity = true;
		m_bHasHadSomeActivity = true;
	}

private:
	Type				m_type;
	State				m_state;
	uint32_t		m_mountStartTime;
	uint32_t		m_unmountStartTime;
	bool				m_bHasActivity;
	bool				m_bHasHadSomeActivity;

	const char 	*m_stateNames[usbsMSCUnmounted+1] = {"CDC", "MSC Initialising", "MSC Mounting", "MSC Mounted", "MSC Unmounting", "MSC Unmounted"};
};



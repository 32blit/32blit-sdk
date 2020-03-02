#include "flash-loader.hpp"
#include "graphics/color.hpp"
#include <cmath>
#include "quadspi.h"
#include "CDCCommandStream.h"
#include "USBManager.h"

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
using namespace blit;

extern QSPI_HandleTypeDef hqspi;
extern CDCCommandStream g_commandStream;

FlashLoader flashLoader;

extern USBManager g_usbManager;

// c calls to c++ object
void init()
{
	flashLoader.Init();
}

void render(uint32_t time)
{
	flashLoader.Render(time);
}

void update(uint32_t time)
{
	flashLoader.Update(time);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Init() Register command handlers
void FlashLoader::Init()
{
	blit::set_screen_mode(ScreenMode::hires);

	// register PROG
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'P', 'R', 'O', 'G'>::value, this);

	// register SAVE
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'S', 'A', 'V', 'E'>::value, this);

	// register LS
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'_', '_', 'L', 'S'>::value, this);

	m_uCurrentFile = persist.selected_menu_item;
}


// FSInit() Read a page worth of *.BIN filenames from the SDCard
void FlashLoader::FSInit(void)
{
	// needed as init is called before sdcard is mounted currently.
	m_uFileCount = 0;

	volatile FRESULT fr;     /* Return value */
	DIR dj;         /* Directory search object */
	FILINFO fno;    /* File information */
	Size s;			/* Size of rendered texts */

	fr = f_findfirst(&dj, &fno, "", "*.BIN");

	while (fr == FR_OK && fno.fname[0] && m_uFileCount < MAX_FILENAMES)
	{
		if(fno.fname[0]!='.')
		{
			strncpy(m_filemeta[m_uFileCount].sFilename, fno.fname, MAX_FILENAME_LENGTH);
			s = screen.measure_text(m_filemeta[m_uFileCount].sFilename, minimal_font, true);
			if (s.w > m_max_width_name) m_max_width_name = s.w;

			m_filemeta[m_uFileCount].fstFilesize = fno.fsize;
			sprintf(m_filemeta[m_uFileCount].sFilesize, "%lu", m_filemeta[m_uFileCount].fstFilesize);
			s = screen.measure_text(m_filemeta[m_uFileCount].sFilesize, minimal_font, true);
			if (s.w > m_max_width_size) m_max_width_size = s.w;

			m_uFileCount++;
		}
		fr = f_findnext(&dj, &fno);
	}

	if(m_uCurrentFile > m_uFileCount) {
		m_uCurrentFile = m_uFileCount;
	}

	f_closedir(&dj);

	m_bFsInit = true;
}


// Flash(): Flash a file from the SDCard to external flash
bool FlashLoader::Flash(const char *pszFilename)
{
	bool bResult = false;

	FIL file;
	FRESULT res = f_open(&file, pszFilename, FA_READ);
	if(!res)
	{
		// get file length
		FSIZE_t uSize = f_size(&file);
		FSIZE_t uTotalBytesRead = 0;
		size_t uOffset = 0;

		if(uSize)
		{
			// quick and dirty erase
			QSPI_WriteEnable(&hqspi);
			qspi_chip_erase();

			bool bFinished = false;

			while(!bFinished)
			{
				// limited ram so a bit at a time
				UINT uBytesRead = 0;
				res = f_read(&file, (void *)m_buffer, BUFFER_SIZE, &uBytesRead);

				if(!res)
				{
					if(uBytesRead)
					{
						if(QSPI_OK == qspi_write_buffer(uOffset, m_buffer, uBytesRead))
						{
							if(QSPI_OK == qspi_read_buffer(uOffset, m_verifyBuffer, uBytesRead))
							{
								// compare buffers
								bool bVerified = true;
								for(uint32_t uB = 0; bVerified && uB < uBytesRead; uB++)
									bVerified = m_buffer[uB] == m_verifyBuffer[uB];

								if(bVerified)
								{
									uOffset += uBytesRead;
									uTotalBytesRead += uBytesRead;
								}
								else
									bFinished = true;
							}
							else
								bFinished = true;
						}
						else
							bFinished = true;
					}

					if(uBytesRead < BUFFER_SIZE)
					{
						bFinished = true;
						bResult = uTotalBytesRead == uSize;
					}
				}
				else
					bFinished = true;
			}
		}

		f_close(&file);
	}

	return bResult;
}


// Render() Call relevant render based on state
void FlashLoader::Render(uint32_t time)
{
	switch(m_state)
	{
		case stFlashFile:
		case stLS:
			RenderFlashFile(time);
			break;

		case stSaveFile:
			RenderSaveFile(time);
			break;

		case stFlashCDC:
			RenderFlashCDC(time);
			break;

		case stSwitch:
			blit::switch_execution();
		break;

		case stMassStorage:
			RenderMassStorage(time);
		break;
	}
}

void FlashLoader::RenderMassStorage(uint32_t time)
{
	static uint8_t uActivityAnim = 0;

	screen.pen = Pen(0,0,0);
	screen.rectangle(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	screen.pen = Pen(255, 255, 255);
	char buffer[128];
	sprintf(buffer, "Mass Storage mode (%s)", g_usbManager.GetStateName());
	screen.text(buffer, minimal_font, ROW(0));

	if(uActivityAnim)
	{
		screen.pen = Pen(0, 255, 0, uActivityAnim);
		screen.circle(Point(320-6, 6), 6);
		uActivityAnim = uActivityAnim>>1;

	}
	else
	{
		if(g_usbManager.HasHadActivity())
			uActivityAnim = 255;
	}


	if(g_usbManager.GetState() == USBManager::usbsMSCUnmounted)
	{
		// Swicth back to CDC
		g_usbManager.SetType(USBManager::usbtCDC);
		FSInit();
		m_state = stFlashFile;
	}
}

// RenderSaveFile() Render file save progress %
void FlashLoader::RenderSaveFile(uint32_t time)
{
	screen.pen = Pen(0,0,0);
	screen.rectangle(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	screen.pen = Pen(255, 255, 255);
	char buffer[128];
	sprintf(buffer, "Saving %.2u%%", (uint16_t)m_fPercent);
	screen.text(buffer, minimal_font, ROW(0));
}


// RenderFlashCDC() Render flashing progress %
void FlashLoader::RenderFlashCDC(uint32_t time)
{
	screen.pen = Pen(0,0,0);
	screen.rectangle(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	screen.pen = Pen(255, 255, 255);

	char buffer[128];
	sprintf(buffer, "Flashing %.2u%%", (uint16_t)m_fPercent);
	screen.text(buffer, minimal_font, ROW(0));
}


// RenderFlashFile() Render main ui for selecting files to flash
void FlashLoader::RenderFlashFile(uint32_t time)
{
	static uint32_t lastRepeat = 0;
	static uint32_t lastButtons = 0;

	if(!m_bFsInit)
		FSInit();

	uint32_t changedButtons = buttons ^ lastButtons;

	bool button_a = buttons & changedButtons & Button::A;
	bool button_x = buttons & changedButtons & Button::X;
	bool button_y = buttons & changedButtons & Button::Y;

	bool button_up = buttons & changedButtons & Button::DPAD_UP;
	bool button_down = buttons & changedButtons & Button::DPAD_DOWN;

	bool button_home = buttons & changedButtons & Button::HOME;

	if(time - lastRepeat > 150 || button_up || button_down) {
		button_up = buttons & Button::DPAD_UP;
		button_down = buttons & Button::DPAD_DOWN;
		lastRepeat = time;
	}

	lastButtons = buttons;

	screen.pen = Pen(0,0,0);
	screen.rectangle(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	screen.pen = Pen(255, 255, 255);

	// just display
	if(m_uFileCount)
	{
		screen.pen = Pen(50, 50, 70);
		screen.rectangle(Rect(0, ROW_HEIGHT*m_uCurrentFile, SCREEN_WIDTH, ROW_HEIGHT));
		screen.pen = Pen(255, 255, 255);

		for(uint8_t uF = 0; uF < m_uFileCount; uF++) {
			// TODO: A single line of text should probably vertically center in a 10px bounding box
			// but in this case it needs to be fudged to 14 pixels
			screen.text(m_filemeta[uF].sFilename, minimal_font, Rect(ROW(uF).x + 5, ROW(uF).y, m_max_width_name, 14), true, TextAlign::center_v);
			screen.line(Point(m_max_width_name + 10, ROW(uF).y), Point(m_max_width_name + 10, ROW(uF).y + 14));
			screen.text(m_filemeta[uF].sFilesize, minimal_font, Rect(m_max_width_name + 16, ROW(uF).y, m_max_width_size, 14), true, TextAlign::center_right);
		}
	}
	else
	{
		screen.text("No Files Found.", minimal_font, ROW(0));
	}

	if(button_home)
	{
		// switch to mass media
		g_usbManager.SetType(USBManager::usbtMSC);
		m_state = stMassStorage;

	}

	if(button_up)
	{
		if(m_uCurrentFile) {
			m_uCurrentFile--;
		} else {
			m_uCurrentFile = MAX_FILENAMES - 1;
		}
	}

	if(button_down)
	{
		if(m_uCurrentFile < m_uFileCount) {
			m_uCurrentFile++;
			m_uCurrentFile %= MAX_FILENAMES;
		}
	}

	if(button_a)
	{
		if(Flash(m_filemeta[m_uCurrentFile].sFilename)) {
			blit::switch_execution();
		}
	}

	if (button_x)
	{
		// Sort by filename
		std::sort(&m_filemeta[0], &m_filemeta[m_uFileCount], [](auto a, auto b) { return strcmp(a.sFilename, b.sFilename) <= 0; });
	}

	if (button_y)
	{
		// Sort by filesize
		std::sort(&m_filemeta[0], &m_filemeta[m_uFileCount], [](auto a, auto b) { return a.fstFilesize <= b.fstFilesize; });
	}

	persist.selected_menu_item = m_uCurrentFile;
}


void FlashLoader::Update(uint32_t time)
{
	if(m_state == stLS)
	{
		FSInit();
		m_state = stFlashFile;
	}
}




//////////////////////////////////////////////////////////////////////
// Streaming Code
//  The streaming code works with a simple state machine,
//  current state is in m_parseState, the states parse index is
//  in m_uParseState
//////////////////////////////////////////////////////////////////////

// StreamInit() Initialise state machine
bool FlashLoader::StreamInit(CDCFourCC uCommand)
{
	//printf("streamInit()\n\r");
	m_fPercent = 0.0f;
	bool bNeedStream = true;
	switch(uCommand)
	{
		case CDCCommandHandler::CDCFourCCMake<'P', 'R', 'O', 'G'>::value:
			m_state = stFlashCDC;
			m_parseState = stFilename;
			m_uParseIndex = 0;
		break;

		case CDCCommandHandler::CDCFourCCMake<'S', 'A', 'V', 'E'>::value:
			m_state = stSaveFile;
			m_parseState = stFilename;
			m_uParseIndex = 0;
		break;

		case CDCCommandHandler::CDCFourCCMake<'_', '_', 'L', 'S'>::value:
			m_state = stLS;
			bNeedStream = false;
		break;

	}
	return bNeedStream;
}


// FlashData() Flash data to the QSPI flash
// Note: currently qspi_write_buffer only works for sizes of 256 max
bool FlashLoader::FlashData(uint32_t uOffset, uint8_t *pBuffer, uint32_t uLen)
{
	bool bResult = false;
	if(QSPI_OK == qspi_write_buffer(uOffset, pBuffer, uLen))
	{
		if(QSPI_OK == qspi_read_buffer(uOffset, m_verifyBuffer, uLen))
		{
			// compare buffers
			bResult = true;

			for(uint32_t uB = 0; bResult && uB < uLen; uB++)
				bResult = pBuffer[uB] == m_verifyBuffer[uB];
		}
	}
	return bResult;
}


// SaveData() Saves date to file on SDCard
bool FlashLoader::SaveData(uint8_t *pBuffer, uint32_t uLen)
{
	UINT uWritten;
	FRESULT res = f_write(&m_file, pBuffer, uLen, &uWritten);

	return !res && (uWritten == uLen);

}


// StreamData() Handle streamed data
// State machine has three states:
// stFilename : Parse filename
// stLength   : Parse length, this is sent as an ascii string
// stData     : The binary data (.bin file)
CDCCommandHandler::StreamResult FlashLoader::StreamData(CDCDataStream &dataStream)
{
	CDCCommandHandler::StreamResult result = srContinue;
	uint8_t byte;
	while(dataStream.GetStreamLength() && result == srContinue)
	{
		switch (m_parseState)
		{
			case stFilename:
				if(m_uParseIndex < MAX_FILENAME)
				{
					while(result == srContinue && m_parseState == stFilename && dataStream.Get(byte))
					{
						m_sFilename[m_uParseIndex++] = byte;
						if (byte == 0)
						{
							m_parseState = stLength;
							m_uParseIndex = 0;
						}
					}
				}
				else
				{
					printf("Failed to read filename\n\r");
					result =srError;
				}
			break;


			case stLength:
				if(m_uParseIndex < MAX_FILELEN)
				{
					while(result == srContinue && m_parseState == stLength && dataStream.Get(byte))
					{
						m_sFilelen[m_uParseIndex++] = byte;
						if (byte == 0)
						{
							m_parseState = stData;
							m_uParseIndex = 0;
							char *pEndPtr;
							m_uFilelen = strtoul(m_sFilelen, &pEndPtr, 10);
							if(m_uFilelen)
							{
								// init file or flash
								switch(m_state)
								{
									case stSaveFile:
									{
										FRESULT res = f_open(&m_file, m_sFilename, FA_CREATE_ALWAYS | FA_WRITE);
										if(res)
										{
											printf("Failed to create file (%s)\n\r", m_sFilename);
											result = srError;
										}
									}
									break;

									case stFlashCDC:
										QSPI_WriteEnable(&hqspi);
										qspi_chip_erase();
									break;

									default:
									break;
								}
							}
							else
							{
								printf("Failed to parse filelen\n\r");
								result =srError;
							}
						}
					}
				}
				else
				{
					printf("Failed to read filelen\n\r");
					result =srError;
				}
			break;

			case stData:
					while((result == srContinue) && (m_parseState == stData) && (m_uParseIndex <= m_uFilelen) && dataStream.Get(byte))
					{
						uint32_t uByteOffset = m_uParseIndex % PAGE_SIZE;
						m_buffer[uByteOffset] = byte;

						// check buffer needs writing
						volatile uint32_t uWriteLen = 0;
						bool bEOS = false;
						if (m_uParseIndex == m_uFilelen-1)
						{
							uWriteLen = uByteOffset+1;
							bEOS = true;
						}
						else
							if(uByteOffset == PAGE_SIZE-1)
								uWriteLen = PAGE_SIZE;

						if(uWriteLen)
						{
							switch(m_state)
							{
								case stSaveFile:
									// save data
									if(!SaveData(m_buffer, uWriteLen))
									{
										printf("Failed to save to SDCard\n\r");
										result = srError;
									}

									// end of stream close up
									if(bEOS)
									{
										f_close(&m_file);
										m_bFsInit = false;
										m_state = stFlashFile;
										if(result != srError)
											result = srFinish;
									}
								break;

								case stFlashCDC:
								{
									// save data
									volatile uint32_t uPage = (m_uParseIndex / PAGE_SIZE);
									if(!FlashData(uPage*PAGE_SIZE, m_buffer, uWriteLen))
									{
										printf("Failed to write to flash\n\r");
										result = srError;
									}

									// end of stream close up
									if(bEOS)
									{
										if(result != srError)
										{
											result = srFinish;
											m_state = stSwitch;
										}
										else
											m_state = stFlashFile;
									}
								}
								break;

								default:
								break;
							}
						}

						m_uParseIndex++;
						m_uBytesHandled = m_uParseIndex;
						m_fPercent = ((float)m_uParseIndex/(float)m_uFilelen)* 100.0f;
					}
			break;
		}
	}

	if(result==srError)
		m_state = stFlashFile;

	return result;
}


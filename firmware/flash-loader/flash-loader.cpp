#include "flash-loader.hpp"
#include "graphics/color.hpp"
#include <cmath>
#include "quadspi.h"
#include "CDCCommandStream.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
using namespace blit;
 
extern CDCCommandStream g_commandStream;

std::vector<FileInfo> files;
int32_t selected_file_index = 0;

std::string progress_message;
int32_t progress = 0;
int32_t progress_total = 0;

FlashLoader flashLoader;

inline bool ends_with(std::string const &value, std::string const &ending)
{
    if(ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void load_file_list() {
  files.clear();
  for(auto file : list_files("")) {
    if(ends_with(file.name, ".bin")) {
      files.push_back(file);
    }
  }
}

Vec2 list_offset(5.0f, 0.0f);


void init()
{
  set_screen_mode(ScreenMode::hires);
  load_file_list();

	// register PROG
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'P', 'R', 'O', 'G'>::value, &flashLoader);
	// register SAVE
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'S', 'A', 'V', 'E'>::value, &flashLoader);
	// register LS
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'_', '_', 'L', 'S'>::value, &flashLoader);
}


void background(uint32_t time_ms) {  
  constexpr float pi = 3.1415927;

  float s = sin(time_ms / 1000.0f) * 5.0f + 20.0f;
  uint32_t o = 0;
  Point p;  
  for (p.y = 0; p.y < 240; p.y++) {
    for (p.x = 0; p.x < 320; p.x++) {
      // TODO: background animation
      /*
      screen.pen.b = 15;
      screen.pen.b += (int((p.x - 160) / s) & 0b111) * 5;
      screen.pen.b += (int((p.y + (list_offset.y * s) - 120) / s) & 0b111) * 5;
      screen.pen.r = 10;
      screen.pen.g = 15;
      screen.pixel(p);      */
      o++;
    }
  }
}

void render(uint32_t time_ms)
{
  

  screen.pen = Pen(20, 30, 40);
  screen.clear();

  background(time_ms);

  for(uint32_t i = 0; i < files.size(); i++) {
    FileInfo *file = &files[i];

    screen.pen = Pen(80, 100, 120);
    if(i == selected_file_index) {
      screen.pen = Pen(235, 245, 255);
    }
    screen.text(file->name, minimal_font, Point(list_offset.x, 115 + i * 10 - list_offset.y));
  }


  screen.text(progress_message, minimal_font, Point(5, 220));
  uint32_t progress_width = ((progress * 310) / progress_total);
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(5, 230, progress_width, 5));

  
//	flashLoader.Render(time);
}

void update(uint32_t time)
{
  static uint32_t last_buttons;
  static uint32_t up_repeat = 0;
  static uint32_t down_repeat = 0;

  up_repeat = (buttons & DPAD_UP) ? up_repeat + 1 : 0;
  down_repeat = (buttons & DPAD_UP) ? down_repeat + 1 : 0;

  bool up_pressed = (buttons & DPAD_UP) && !(last_buttons & DPAD_UP);
  bool down_pressed = (buttons & DPAD_DOWN) && !(last_buttons & DPAD_DOWN);
  bool a_pressed = (buttons & A) && !(last_buttons & A);

  // handle up/down clicks to select files in the list
  if(up_pressed)    { selected_file_index--; }
  if(down_pressed)  { selected_file_index++; }
  int32_t file_count = files.size();
  selected_file_index = ((selected_file_index % file_count) + file_count) % file_count;

  // scroll list towards selected item  
  list_offset.y += ((selected_file_index * 10) - list_offset.y) / 5.0f;

  // select current item in list to launch
  if(a_pressed) {
    flash_from_sd_to_qspi_flash(files[selected_file_index].name);
    blit_switch_execution();
  }

  

	//flashLoader.Update(time);


	// register LS
//	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'_', '_', 'L', 'S'>::value, this);

//	m_uCurrentFile = persist.selected_menu_item;
  last_buttons = buttons;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool flash_from_sd_to_qspi_flash(const std::string &filename)
{  
  static uint8_t file_buffer[4 * 1024];

	FIL file;
	if(f_open(&file, filename.c_str(), FA_READ) != FR_OK) {
		return false;
	}

  UINT bytes_total = f_size(&file);
  UINT bytes_flashed  = 0;

  progress_message = "Erasing sectors...";
  progress = 0;
  progress_total = bytes_total / (64 * 1024);
  
  // TODO: don't erase entire flash chip...  
  for(UINT sector_start = 0; sector_start < bytes_total; sector_start += 64 * 1024) {
    qspi_sector_erase(sector_start);
    progress++;
    render_yield();
  }
  
  

  progress_message = "Copying to external flash...";
  progress = 0;
  progress_total = bytes_total;

  //render_yield(true);
  
  while(bytes_flashed < bytes_total) {
    UINT bytes_read = 0;
    
    // read up to 4KB from the file on sd card
    if(f_read(&file, (void *)file_buffer, sizeof(file_buffer), &bytes_read) != FR_OK) {
      return false;
    }

    // write the read data to the external qspi flash
    if(qspi_write_buffer(bytes_flashed, file_buffer, bytes_read) != QSPI_OK) {
      return false;
    }

    // TODO: is it worth reading the data back and performing a verify here? not sure...

    bytes_flashed += bytes_read;

    progress = bytes_flashed;

    render_yield();
  }

  f_close(&file);

  progress_message = "";

	return true;
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
			blit_switch_execution();
		break;
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


	uint32_t changedButtons = buttons ^ lastButtons;

	bool button_a = buttons & changedButtons & Button::A;

	bool button_up = buttons & changedButtons & Button::DPAD_UP;
	bool button_down = buttons & changedButtons & Button::DPAD_DOWN;

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
	if(files.size() > 0)
	{
		screen.pen = Pen(50, 50, 70);
		screen.rectangle(Rect(0, ROW_HEIGHT*m_uCurrentFile, SCREEN_WIDTH, ROW_HEIGHT));
		screen.pen = Pen(255, 255, 255);

		for(uint8_t uF = 0; uF < files.size(); uF++) {
			// TODO: A single line of text should probably vertically center in a 10px bounding box
			// but in this case it needs to be fudged to 14 pixels
			screen.text(files[uF].name, minimal_font, Rect(ROW(uF).x + 5, ROW(uF).y, 310, 14), true, TextAlign::center_v);
		}
	}
	else
	{
		screen.text("No Files Found.", minimal_font, ROW(0));
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
    std::string filename = files[m_uCurrentFile].name;
		if(flash_from_sd_to_qspi_flash(filename)) {
			blit_switch_execution();
		}
	}

	persist.selected_menu_item = m_uCurrentFile;
}


void FlashLoader::Update(uint32_t time)
{
	if(m_state == stLS)
	{
		load_file_list();
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


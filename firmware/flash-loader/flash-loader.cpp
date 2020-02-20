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

constexpr uint32_t qspi_flash_sector_size = 64 * 1024;

std::vector<FileInfo> files;
Vec2 file_list_scroll_offset(5.0f, 0.0f);

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
    if(i == persist.selected_menu_item) {
      screen.pen = Pen(235, 245, 255);
    }
    screen.text(file->name, minimal_font, Point(file_list_scroll_offset.x, 115 + i * 10 - file_list_scroll_offset.y));
  }


  progress.draw();

  
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
  if(up_pressed)    { persist.selected_menu_item--; }
  if(down_pressed)  { persist.selected_menu_item++; }
  int32_t file_count = files.size();
  persist.selected_menu_item = ((persist.selected_menu_item % file_count) + file_count) % file_count;

  // scroll list towards selected item  
  file_list_scroll_offset.y += ((persist.selected_menu_item * 10) - file_list_scroll_offset.y) / 5.0f;

  // select current item in list to launch
  if(a_pressed) {
    flash_from_sd_to_qspi_flash(files[persist.selected_menu_item].name);
    blit_switch_execution();
  }

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

  // erase the sectors needed to write the image  
  uint32_t sector_count = (bytes_total / qspi_flash_sector_size) + 1;

  progress.show("Erasing sectors...", sector_count);

  for(uint32_t sector = 0; sector < sector_count; sector++) {
    qspi_sector_erase(sector * qspi_flash_sector_size);

    progress.update(sector);    
  }

  // read the image from as card and write it to the qspi flash  
  progress.show("Copying to external flash...", bytes_total);

  UINT bytes_flashed  = 0;
  
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

    bytes_flashed += bytes_read;

    // TODO: is it worth reading the data back and performing a verify here? not sure...

    progress.update(bytes_flashed);
  }

  f_close(&file);

  progress.hide();

	return true;
}


/*
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
*/

/*
void FlashLoader::Update(uint32_t time)
{
	if(m_state == stLS)
	{
		load_file_list();
		m_state = stFlashFile;
	}
}
*/



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
/*
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
*/

// SaveData() Saves date to file on SDCard
bool FlashLoader::SaveData(uint8_t *pBuffer, uint32_t uLen)
{
	UINT uWritten;
	FRESULT res = f_write(&m_file, pBuffer, uLen, &uWritten);

	return !res && (uWritten == uLen);

}

void copy_from_usb_to_sd_card() {

}

// StreamData() Handle streamed data
// State machine has three states:
// stFilename : Parse filename
// stLength   : Parse length, this is sent as an ascii string
// stData     : The binary data (.bin file)


// when a command is issued (e.g. "PROG" or "SAVE") then this function is called
// whenever new data is received to allow the firmware to process it.
CDCCommandHandler::StreamResult FlashLoader::StreamData(CDCDataStream &stream)
{
  enum class StreamState { NONE, NAME, LENGTH, DATA };
  static StreamState state = StreamState::NONE;

  static char   name_buffer[256];
  static char length_buffer[ 16];
  static char   data_buffer[2048];

  static FIL  file;
  static UINT bytes_total;
  static UINT bytes_read = 0;

  static char *p = nullptr;  

  if(state == StreamState::NONE) {
    p = name_buffer;
    state = StreamState::NAME;         
  }

  // loop through all bytes in the input stream
  uint8_t byte;
  while(stream.Get(byte)) {        
    *p++ = byte;

    switch(state) {
      // sending filename
      case StreamState::NAME:
        if(byte == '\0') {        
          p = length_buffer;
          state = StreamState::LENGTH;
        }
      
        break;

      // sending file length
      case StreamState::LENGTH:
        if(byte == '\0') {        
          bytes_read = 0;
          bytes_total = atol(length_buffer);
          p = data_buffer;          

          blit::LED.r = 255;  
          blit_update_led();

          if(f_open(&file, name_buffer, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
            progress.show(std::to_string(srError), 1);
            return srError;
          }

          state = StreamState::DATA;

          progress.show("Copying to SD card...", bytes_total);
        }
      
        break;

      // sending file data
      case StreamState::DATA:
        bytes_read++;
        // completed the read or filled the buffer? write to the file
        if(bytes_read == bytes_total || p == (data_buffer + sizeof(data_buffer))) {
          UINT bytes_written;
          if(f_write(&file, data_buffer, p - data_buffer, &bytes_written) != FR_OK) {
            return srError;
          }
          p = data_buffer;
          progress.update(bytes_read);
        }        

        // completed the write? close the file
        if(bytes_read == bytes_total) {
          f_close(&file);
          state = StreamState::NONE;
          progress.hide();
          
          return srFinish;
        }
        
        break;
    }
  }

  return srContinue;

  /*
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
		m_state = stFlashFile;*/

	//return result;
}


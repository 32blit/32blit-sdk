#include "flash-loader.hpp"
#include "graphics/color.hpp"
#include <cmath>
#include "quadspi.h"
#include "CDCCommandStream.h"
#include "USBManager.h"
#include "file.hpp"

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
using namespace blit;

constexpr uint32_t qspi_flash_sector_size = 64 * 1024;

Vec2 file_list_scroll_offset(20.0f, 0.0f);

extern CDCCommandStream g_commandStream;

FlashLoader flashLoader;

extern USBManager g_usbManager;

void erase_qspi_flash(uint32_t start_sector, uint32_t size_bytes) {
  uint32_t sector_count = (size_bytes / qspi_flash_sector_size) + 1;

  progress.show("Erasing flash sectors...", sector_count);

  for(uint32_t sector = 0; sector < sector_count; sector++) {
    qspi_sector_erase((start_sector + sector) * qspi_flash_sector_size);

    progress.update(sector);
  }

  progress.hide();
}

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
}


// FSInit() Read a page worth of *.BIN filenames from the SDCard
void FlashLoader::FSInit(void)
{
  m_filemeta.clear();
  m_max_width_size = 0;

  for(auto &file : ::list_files("/"))
  {
    if(file.flags & blit::FileFlags::directory)
      continue;

    if(file.name.length() < 4)
      continue;

    if(file.name.compare(file.name.length() - 4, 4, ".bin") == 0 || file.name.compare(file.name.length() - 4, 4, ".BIN") == 0)
    {
      m_filemeta.push_back(file);
      m_max_width_size = std::max(m_max_width_size, screen.measure_text(std::to_string(file.size), minimal_font).w);
    }
  }

  if(persist.selected_menu_item > m_filemeta.size()) {
    persist.selected_menu_item = m_filemeta.size() - 1;
  }

  m_bFsInit = true;
}


// Flash(): Flash a file from the SDCard to external flash
bool FlashLoader::Flash(const char *pszFilename)
{
  FIL file;
  FRESULT res = f_open(&file, pszFilename, FA_READ);
  if(res != FR_OK)
    return false;

  // get file length
  FSIZE_t bytes_total = f_size(&file);
  FSIZE_t bytes_flashed = 0;
  size_t offset = 0;

  if(!bytes_total)
  {
    f_close(&file);
    return false;
  }

  // erase the sectors needed to write the image
  erase_qspi_flash(0, bytes_total);

  progress.show("Copying from SD card to flash...", bytes_total);

  while(bytes_flashed < bytes_total)
  {
    // limited ram so a bit at a time
    UINT bytes_read = 0;
    res = f_read(&file, (void *)m_buffer, BUFFER_SIZE, &bytes_read);

    if(res != FR_OK)
      break;
  
    if(qspi_write_buffer(offset, m_buffer, bytes_read) != QSPI_OK)
      break;

    if(qspi_read_buffer(offset, m_verifyBuffer, bytes_read) != QSPI_OK)
      break;

    // compare buffers
    bool verified = true;
    for(uint32_t uB = 0; verified && uB < bytes_read; uB++)
      verified = m_buffer[uB] == m_verifyBuffer[uB];

    if(!verified)
      break;

    offset += bytes_read;
    bytes_flashed += bytes_read;

    progress.update(bytes_flashed);
  }

  f_close(&file);

  progress.hide();

  return bytes_flashed == bytes_total;
}

void FlashLoader::Render(uint32_t time) {
  screen.pen = Pen(5, 8, 12);
  screen.clear();

  screen.pen = Pen(0, 0, 0, 100);
  screen.rectangle(Rect(10, 0, 100, 240));

  // list files on SD card
  if(!m_filemeta.empty()) {
    int y = 115 - file_list_scroll_offset.y;
    // adjust alignment rect for vertical spacing
    const int text_align_height = ROW_HEIGHT + minimal_font.spacing_y;
    const int size_x = 115;
    
    uint32_t i = 0;
    for(auto &file : m_filemeta) {
      if(i++ == persist.selected_menu_item)
        screen.pen = Pen(235, 245, 255);
      else
        screen.pen = Pen(80, 100, 120);

      screen.text(file.name, minimal_font, Rect(file_list_scroll_offset.x, y, 100 - 20, text_align_height), true, TextAlign::center_v);
      screen.line(Point(size_x - 4, y), Point(size_x - 4, y + ROW_HEIGHT));
      screen.text(std::to_string(file.size), minimal_font, Rect(size_x, y, m_max_width_size, text_align_height), true, TextAlign::center_right);
      y += ROW_HEIGHT;
    }
  }
  else {
    screen.text("No Files Found.", minimal_font, ROW(0));
  }

  if(m_state == stMassStorage)
    RenderMassStorage(time);

  progress.draw();
}

void FlashLoader::RenderMassStorage(uint32_t time)
{
  static uint8_t uActivityAnim = 0;

  screen.pen = Pen(0, 0, 0, 200);
  screen.clear();

  screen.pen = Pen(255, 255, 255);
  char buffer[128];
  sprintf(buffer, "Mass Storage mode (%s)", g_usbManager.GetStateName());
  screen.text(buffer, minimal_font, Rect(Point(0), screen.bounds), true, TextAlign::center_center);

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
}

void FlashLoader::Update(uint32_t time)
{
  if(m_state == stLS) {
    FSInit();
    m_state = stFlashFile;
  }
  else if(m_state == stSwitch) {
    blit_switch_execution();
    return; // not reached
  }

  bool button_home = buttons.pressed & Button::HOME;
  
  if(m_state == stFlashFile)
  {
    static uint32_t lastRepeat = 0;

    if(!m_bFsInit)
      FSInit();

    bool button_a = buttons.pressed & Button::A;
    bool button_x = buttons.pressed & Button::X;
    bool button_y = buttons.pressed & Button::Y;

    bool button_up = buttons.pressed & Button::DPAD_UP;
    bool button_down = buttons.pressed & Button::DPAD_DOWN;

    if(time - lastRepeat > 150 || button_up || button_down) {
      button_up = buttons & Button::DPAD_UP;
      button_down = buttons & Button::DPAD_DOWN;
      lastRepeat = time;
    }

    if(button_home)
    {
      // switch to mass media
      g_usbManager.SetType(USBManager::usbtMSC);
      m_state = stMassStorage;

    }

    if(button_up)
    {
      if(persist.selected_menu_item > 0) {
        persist.selected_menu_item--;
      } else {
        persist.selected_menu_item = m_filemeta.size() - 1;
      }
    }

    if(button_down)
    {
      if(persist.selected_menu_item < (m_filemeta.size() - 1)) {
        persist.selected_menu_item++;
      } else {
        persist.selected_menu_item = 0;
      }
    }

    // scroll list towards selected item  
    file_list_scroll_offset.y += ((persist.selected_menu_item * 10) - file_list_scroll_offset.y) / 5.0f;

    if(button_a)
    {
      if(Flash(m_filemeta[persist.selected_menu_item].name.c_str())) {
        blit_switch_execution();
      }
    }

    using Iterator = std::vector<FileInfo>::iterator;
    using Compare = bool(const FileInfo &, const FileInfo &);
    if (button_x)
    {
      // Sort by filename
      std::sort<Iterator, Compare>(m_filemeta.begin(), m_filemeta.end(), [](const auto &a, const auto &b) { return a.name < b.name; });
    }

    if (button_y)
    {
      // Sort by filesize
      std::sort<Iterator, Compare>(m_filemeta.begin(), m_filemeta.end(), [](const auto &a, const auto &b) { return a.size < b.size; });
    }
  }
  else if(m_state == stMassStorage)
  {
    bool switch_back = g_usbManager.GetState() == USBManager::usbsMSCUnmounted;

    // allow switching back manually if it was never mounted
    if(button_home && g_usbManager.GetState() == USBManager::usbsMSCInititalising)
      switch_back = true;

    if(switch_back)
    {
      // Switch back to CDC
      g_usbManager.SetType(USBManager::usbtCDC);
      FSInit();
      m_state = stFlashFile;
    }
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

  progress.update(uOffset + uLen);
  return bResult;
}


// SaveData() Saves date to file on SDCard
bool FlashLoader::SaveData(uint8_t *pBuffer, uint32_t uLen)
{
  UINT uWritten;
  FRESULT res = f_write(&m_file, pBuffer, uLen, &uWritten);

  progress.update(f_tell(&m_file));

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
                    else
                      progress.show("Saving " + std::string(m_sFilename) +  " to SD card...", m_uFilelen);
                  }
                  break;

                  case stFlashCDC:
                    erase_qspi_flash(0, m_uFilelen);
                    progress.show("Saving " + std::string(m_sFilename) +  " to flash...", m_uFilelen);
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

                    progress.hide();
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

                    progress.hide();
                  }
                }
                break;

                default:
                break;
              }
            }

            m_uParseIndex++;
            m_uBytesHandled = m_uParseIndex;
          }
      break;
    }
  }

  if(result == srError) {
    m_state = stFlashFile;
    progress.hide();
  }

  return result;
}


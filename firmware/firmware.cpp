#include "firmware.hpp"
#include "graphics/color.hpp"
#include <cmath>
#include "quadspi.h"
#include "CDCCommandStream.h"
#include "USBManager.h"
#include "file.hpp"
#include "executable.hpp"
#include "metadata.hpp"

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
using namespace blit;

enum State {stFlashFile, stSaveFile, stFlashCDC, stLS, stMassStorage};

constexpr uint32_t qspi_flash_sector_size = 64 * 1024;
constexpr uint32_t qspi_flash_size = 32768 * 1024;

Vec2 file_list_scroll_offset(20.0f, 0.0f);

extern CDCCommandStream g_commandStream;

FlashLoader flashLoader;

extern USBManager g_usbManager;

struct GameInfo {
  std::string title;
  uint32_t size;

  std::string filename; // if on SD
  uint32_t offset; // in in flash
};

std::string current_directory = "/";
std::vector<GameInfo> flash_games, sd_games;
std::vector<std::string> directory_list;

int current_directory_index = 0;

bool display_flash = true;

SortBy file_sort = SortBy::name;

uint8_t buffer[PAGE_SIZE];
uint8_t verify_buffer[PAGE_SIZE];

State		state = stFlashFile;

FIL file;


BlitGameMetadata selected_game_metadata;

// error dialog
int selected_dialog_option = 0;

uint32_t flash_from_sd_to_qspi_flash(const char *filename);

int calc_num_blocks(uint32_t size) {
  return (size - 1) / qspi_flash_sector_size + 1;
}

void erase_qspi_flash(uint32_t start_sector, uint32_t size_bytes) {
  uint32_t sector_count = (size_bytes / qspi_flash_sector_size) + 1;

  progress.show("Erasing flash sectors...", sector_count);

  for(uint32_t sector = 0; sector < sector_count; sector++) {
    qspi_sector_erase((start_sector + sector) * qspi_flash_sector_size);

    progress.update(sector);
  }

  progress.hide();
}

void sort_file_list() {
    using Iterator = std::vector<GameInfo>::iterator;
    using Compare = bool(const GameInfo &, const GameInfo &);

    if (file_sort == SortBy::name) {
      // Sort by filename
      std::sort<Iterator, Compare>(flash_games.begin(), flash_games.end(), [](const auto &a, const auto &b) { return a.title < b.title; });
      std::sort<Iterator, Compare>(sd_games.begin(), sd_games.end(), [](const auto &a, const auto &b) { return a.title < b.title; });
    }

    if (file_sort == SortBy::size) {
      // Sort by filesize
      std::sort<Iterator, Compare>(flash_games.begin(), flash_games.end(), [](const auto &a, const auto &b) { return a.size < b.size; });
      std::sort<Iterator, Compare>(sd_games.begin(), sd_games.end(), [](const auto &a, const auto &b) { return a.size < b.size; });
    }
}

void load_directory_list(std::string directory) {
  directory_list.clear();

  for(auto &folder : ::list_files(directory)) {
    if(folder.flags & blit::FileFlags::directory) {
      if(folder.name.compare("System Volume Information") == 0) continue;
      directory_list.push_back(folder.name);
    }
  }

  using Iterator = std::vector<std::string>::iterator;
  using Compare = bool(const std::string &, const std::string &);

  std::sort<Iterator, Compare>(directory_list.begin(), directory_list.end(), [](const auto &a, const auto &b) { return a > b; });

  directory_list.push_back("FLASH");
}

void load_file_list(std::string directory) {
  sd_games.clear();

  for(auto &file : ::list_files(directory)) {
    if(file.flags & blit::FileFlags::directory)
      continue;

    if(file.name.length() < 4)
      continue;

    if(file.name.compare(file.name.length() - 4, 4, ".bin") == 0 || file.name.compare(file.name.length() - 4, 4, ".BIN") == 0) {

      GameInfo game;
      game.title = file.name.substr(0, file.name.length() - 4);
      game.filename = directory + "/" + file.name;
      game.size = file.size;
      
      // check for metadata
      BlitGameMetadata meta;
      if(parse_file_metadata(file.name, meta))
        game.title = meta.title;

      sd_games.push_back(game);
    }
  }

  sort_file_list();
}

void scan_flash() {
  flash_games.clear();

  for(uint32_t offset = 0; offset < qspi_flash_size;) {
    BlitGameHeader header;

    if(qspi_read_buffer(offset, reinterpret_cast<uint8_t *>(&header), sizeof(header)) != QSPI_OK)
      break;

    if(header.magic != blit_game_magic) {
      offset += qspi_flash_sector_size;
      continue;
    }

    GameInfo game;
    game.offset = offset;
    game.size = header.end - 0x90000000;
    game.title = "game @" + std::to_string(game.offset / qspi_flash_sector_size);

    // check for valid metadata
    BlitGameMetadata meta;
    if(parse_flash_metadata(offset, meta)) {
      game.title = meta.title;
      game.size += meta.length + 10;
    }

    flash_games.push_back(game);

    offset += calc_num_blocks(game.size);
  }
}

void load_current_game_metadata() {
  auto &games = current_directory.compare("FLASH") == 0 ? flash_games : sd_games;

  auto &game = games[persist.selected_menu_item];
  bool loaded;
  if(game.filename.empty())
    loaded = parse_flash_metadata(game.offset, selected_game_metadata, true);
  else
    loaded = parse_file_metadata(game.filename, selected_game_metadata, true);

  // no valid metadata, reset
  if(!loaded) {
    selected_game_metadata.free_surfaces();
    selected_game_metadata = BlitGameMetadata();
  }
}

void mass_storage_overlay(uint32_t time)
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

void init() {
  set_screen_mode(ScreenMode::hires);
  screen.clear();

  scan_flash();
  load_directory_list("/");
  current_directory = directory_list.empty() ? "/" : directory_list.front();
  load_file_list(current_directory);

  auto &games = current_directory.compare("FLASH") == 0 ? flash_games : sd_games;

  auto total_items = games.size();
  if(persist.selected_menu_item > total_items)
    persist.selected_menu_item = total_items - 1;

  load_current_game_metadata();

  // register PROG
  g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'P', 'R', 'O', 'G'>::value, &flashLoader);

  // register SAVE
  g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'S', 'A', 'V', 'E'>::value, &flashLoader);

  // register LS
  g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'_', '_', 'L', 'S'>::value, &flashLoader);
}

void render(uint32_t time) {
  screen.pen = Pen(5, 8, 12);
  screen.clear();

  screen.pen = Pen(0, 0, 0, 100);
  screen.rectangle(Rect(10, 0, 100, 240));

  // adjust alignment rect for vertical spacing
  const int text_align_height = ROW_HEIGHT + minimal_font.spacing_y;

  int x = 120;

  // list folders
  if(!directory_list.empty()) {
    for(auto &directory : directory_list) {
      if(directory.compare(current_directory) == 0)
        screen.pen = Pen(235, 245, 255);
      else
        screen.pen = Pen(80, 100, 120);

      auto text_width = screen.measure_text(directory, minimal_font, true);
      screen.text(directory, minimal_font, Rect(x, 5, 100 - 20, text_align_height), true, TextAlign::center_v);
      x += text_width.w + 10;
    }
  }

  int y = 115 - file_list_scroll_offset.y;
  uint32_t i = 0;

  // list games
  auto &games = current_directory.compare("FLASH") == 0 ? flash_games : sd_games;
  if(!games.empty()) {
    const int size_x = 115;

    for(auto &file : games) {
      if(i++ == persist.selected_menu_item)
        screen.pen = Pen(235, 245, 255);
      else
        screen.pen = Pen(80, 100, 120);

      screen.text(file.title, minimal_font, Rect(file_list_scroll_offset.x, y, 100 - 20, text_align_height), true, TextAlign::center_v);
      y += ROW_HEIGHT;
    }
  }
  else {
    screen.pen = Pen(235, 245, 255);
    screen.text("No Files Found.", minimal_font, Point(20, screen.bounds.h / 2), true, TextAlign::center_v);
  }

  // game info

  if(selected_game_metadata.splash)
    screen.blit(selected_game_metadata.splash, Rect(Point(0, 0), selected_game_metadata.splash->bounds), Point(172, 20));

  screen.pen = Pen(235, 245, 255);
  screen.text(selected_game_metadata.title, minimal_font, Point(172, 124));

  Rect desc_rect(172, 138, 128, 72);

  screen.pen = Pen(80, 100, 120);
  std::string wrapped_desc = screen.wrap_text(selected_game_metadata.description, desc_rect.w, minimal_font);
  screen.text(wrapped_desc, minimal_font, desc_rect);

  int num_blocks = calc_num_blocks(games[persist.selected_menu_item].size);
  char buf[20];
  snprintf(buf, 20, "%i block%s", num_blocks, num_blocks == 1 ? "" : "s");
  screen.text(buf, minimal_font, Point(172, 216));

  // overlays
  if(state == stMassStorage)
    mass_storage_overlay(time);

  // error dialog overlay
  if(persist.reset_error) {
    screen.pen = Pen(0, 0, 0, 200);
    screen.clear();

    screen.pen = Pen(255, 255, 255);

    screen.text("Oops!\n\nRestart game?", minimal_font, Point(screen.bounds.w / 2, screen.bounds.h / 2), true, TextAlign::center_center);

    screen.pen = selected_dialog_option == 0 ? Pen(235, 245, 255) : Pen(80, 100, 120);
    screen.text("No", minimal_font, Point(screen.bounds.w / 3, screen.bounds.h - 80), true, TextAlign::center_center);

    screen.pen = selected_dialog_option == 1 ? Pen(235, 245, 255) : Pen(80, 100, 120);
    screen.text("Yes", minimal_font, Point(screen.bounds.w / 3 * 2, screen.bounds.h - 80), true, TextAlign::center_center);
  }

  progress.draw();
}

void update(uint32_t time)
{
  if(persist.reset_error) {
    // only two options
    if(buttons.pressed & Button::DPAD_RIGHT)
      selected_dialog_option ^= 1;
    else if(buttons.pressed & Button::DPAD_LEFT)
      selected_dialog_option ^= 1;

    if(buttons.released & Button::A) {
      if(selected_dialog_option == 1) // yes
        blit_switch_execution(0);
      else
        persist.reset_target = prtFirmware;

      persist.reset_error = false;
    }

    return;
  }

  if(state == stLS) {
    load_file_list(current_directory);
    state = stFlashFile;
  }

  bool button_home = buttons.pressed & Button::HOME;
  
  if(state == stFlashFile)
  {
    static uint32_t lastRepeat = 0;

    bool button_a = buttons.released & Button::A;
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
      state = stMassStorage;

    }

    //auto &games = display_flash ? flash_games : sd_games;
    auto &games = current_directory.compare("FLASH") == 0 ? flash_games : sd_games;
    auto total_items = games.size();

    auto old_menu_item = persist.selected_menu_item;

    if(button_up)
    {
      if(persist.selected_menu_item > 0) {
        persist.selected_menu_item--;
      } else {
        persist.selected_menu_item = total_items - 1;
      }
    }

    if(button_down)
    {
      if(persist.selected_menu_item < (total_items - 1)) {
        persist.selected_menu_item++;
      } else {
        persist.selected_menu_item = 0;
      }
    }

    // switch between flash and SD lists
    if(buttons.pressed & Button::DPAD_LEFT) {
      current_directory_index--;
      if(current_directory_index < 0) {
        current_directory_index = directory_list.size() - 1;
      }
    }
    if(buttons.pressed & Button::DPAD_RIGHT) {
      //display_flash = !display_flash;
      current_directory_index++;
      if((uint32_t)current_directory_index >= directory_list.size()) {
        current_directory_index = 0;
      }
    }
    if(buttons.pressed & (Button::DPAD_LEFT | Button::DPAD_RIGHT)) {
      current_directory = directory_list[current_directory_index];
      load_file_list(current_directory);
      persist.selected_menu_item = 0;
      load_current_game_metadata();
    }

    // scroll list towards selected item  
    file_list_scroll_offset.y += ((persist.selected_menu_item * 10) - file_list_scroll_offset.y) / 5.0f;

    // load metadata for selected item
    if(persist.selected_menu_item != old_menu_item)
      load_current_game_metadata();

    if(button_a)
    {
      uint32_t offset;
      auto &game = games[persist.selected_menu_item];

      if(game.filename.empty())
        offset = games[persist.selected_menu_item].offset; // flash
      else
        offset = flash_from_sd_to_qspi_flash(game.filename.c_str()); // sd

      if(offset != 0xFFFFFFFF)
        blit_switch_execution(offset);
    }

    if (button_x) {
      file_sort = SortBy::name;
      sort_file_list();
    }

    if (button_y) {
      file_sort = SortBy::size;
      sort_file_list();
    }
  }
  else if(state == stMassStorage)
  {
    bool switch_back = g_usbManager.GetState() == USBManager::usbsMSCUnmounted;

    // allow switching back manually if it was never mounted
    if(button_home && g_usbManager.GetState() == USBManager::usbsMSCInititalising)
      switch_back = true;

    if(switch_back)
    {
      // Switch back to CDC
      g_usbManager.SetType(USBManager::usbtCDC);
      load_file_list(current_directory);
      state = stFlashFile;
    }
  }
}

// returns address to flash file to
uint32_t get_flash_offset_for_file(BlitGameHeader &bin_header) {

  // temporary load address for working on multiple app support without PIC being ready
  // in future this will probably be more of a "find free space" function
  if(bin_header.magic == blit_game_magic) {
    auto expected_addr = bin_header.start;

    // this should be sector aligned to not break things later...
    return expected_addr - 0x90000000;
  }

  return 0;
}

// Flash(): Flash a file from the SDCard to external flash
uint32_t flash_from_sd_to_qspi_flash(const char *filename)
{
  FIL file;
  FRESULT res = f_open(&file, filename, FA_READ);
  if(res != FR_OK)
    return false;

  // get file length
  FSIZE_t bytes_total = f_size(&file);
  UINT bytes_read = 0;
  FSIZE_t bytes_flashed = 0;
  size_t offset = 0;

  if(!bytes_total)
  {
    f_close(&file);
    return false;
  }

  // check header
  BlitGameHeader header;
  if(f_read(&file, (void *)&header, sizeof(header), &bytes_read) != FR_OK) {
    f_close(&file);
    return false;
  }
  f_lseek(&file, 0);

  uint32_t flash_offset = get_flash_offset_for_file(header);
  offset = flash_offset;

  // erase the sectors needed to write the image
  erase_qspi_flash(flash_offset / qspi_flash_sector_size, bytes_total);

  progress.show("Copying from SD card to flash...", bytes_total);

  while(bytes_flashed < bytes_total)
  {
    // limited ram so a bit at a time
    res = f_read(&file, (void *)buffer, BUFFER_SIZE, &bytes_read);

    if(res != FR_OK)
      break;
  
    if(qspi_write_buffer(offset, buffer, bytes_read) != QSPI_OK)
      break;

    if(qspi_read_buffer(offset, verify_buffer, bytes_read) != QSPI_OK)
      break;

    // compare buffers
    bool verified = true;
    for(uint32_t uB = 0; verified && uB < bytes_read; uB++)
      verified = buffer[uB] == verify_buffer[uB];

    if(!verified)
      break;

    offset += bytes_read;
    bytes_flashed += bytes_read;

    progress.update(bytes_flashed);
  }

  f_close(&file);

  progress.hide();

  return bytes_flashed == bytes_total ? flash_offset : 0xFFFFFFFF;
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
      state = stFlashCDC;
      m_parseState = stFilename;
      m_uParseIndex = 0;
    break;

    case CDCCommandHandler::CDCFourCCMake<'S', 'A', 'V', 'E'>::value:
      state = stSaveFile;
      m_parseState = stFilename;
      m_uParseIndex = 0;
    break;

    case CDCCommandHandler::CDCFourCCMake<'_', '_', 'L', 'S'>::value:
      state = stLS;
      bNeedStream = false;
    break;

  }
  return bNeedStream;
}


// FlashData() Flash data to the QSPI flash
// Note: currently qspi_write_buffer only works for sizes of 256 max
bool FlashData(uint32_t uOffset, uint8_t *pBuffer, uint32_t uLen)
{
  bool bResult = false;
  if(QSPI_OK == qspi_write_buffer(uOffset, pBuffer, uLen))
  {
    if(QSPI_OK == qspi_read_buffer(uOffset, verify_buffer, uLen))
    {
      // compare buffers
      bResult = true;

      for(uint32_t uB = 0; bResult && uB < uLen; uB++)
        bResult = pBuffer[uB] == verify_buffer[uB];
    }
  }

  progress.update(uOffset + uLen);
  return bResult;
}


// SaveData() Saves date to file on SDCard
bool SaveData(uint8_t *pBuffer, uint32_t uLen)
{
  UINT uWritten;
  FRESULT res = f_write(&file, pBuffer, uLen, &uWritten);

  progress.update(f_tell(&file));

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
                switch(state)
                {
                  case stSaveFile:
                  {
                    FRESULT res = f_open(&file, m_sFilename, FA_CREATE_ALWAYS | FA_WRITE);
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
            buffer[uByteOffset] = byte;

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
              switch(state)
              {
                case stSaveFile:
                  // save data
                  if(!SaveData(buffer, uWriteLen))
                  {
                    printf("Failed to save to SDCard\n\r");
                    result = srError;
                  }

                  // end of stream close up
                  if(bEOS)
                  {
                    f_close(&file);
                    load_file_list(current_directory);
                    state = stFlashFile;
                    if(result != srError)
                      result = srFinish;

                    progress.hide();
                  }
                break;

                case stFlashCDC:
                {
                  uint32_t uPage = (m_uParseIndex / PAGE_SIZE);
                  // first page, check header
                  if(uPage == 0) {
                    flash_start_offset = get_flash_offset_for_file(*reinterpret_cast<BlitGameHeader *>(buffer));

                    // erase
                    erase_qspi_flash(flash_start_offset / qspi_flash_sector_size, m_uFilelen);
                    progress.show("Saving " + std::string(m_sFilename) +  " to flash...", m_uFilelen);
                  }

                  // save data
                  if(!FlashData(flash_start_offset + uPage*PAGE_SIZE, buffer, uWriteLen))
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
                      blit_switch_execution(flash_start_offset);
                    }
                    else
                      state = stFlashFile;

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
    state = stFlashFile;
    progress.hide();
  }

  return result;
}


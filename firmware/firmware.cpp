#include "firmware.hpp"
#include "graphics/color.hpp"
#include <cmath>
#include "quadspi.h"
#include "CDCCommandStream.h"
#include "USBManager.h"
#include "usbd_cdc_if.h"
#include "file.hpp"
#include "executable.hpp"
#include "metadata.hpp"
#include "dialog.hpp"
#include "engine/api_private.hpp"

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <list>

using namespace blit;

enum State {stFlashFile, stSaveFile, stFlashCDC, stMassStorage};

constexpr uint32_t qspi_flash_sector_size = 64 * 1024;
constexpr uint32_t qspi_flash_size = 32768 * 1024;
constexpr uint32_t qspi_flash_address = 0x90000000;

extern CDCCommandStream g_commandStream;

FlashLoader flashLoader;
CDCEraseHandler cdc_erase_handler;

extern USBManager g_usbManager;

struct GameInfo {
  std::string title, author;
  uint32_t size, checksum = 0;

  uint32_t offset;
};

std::vector<GameInfo> game_list;

std::list<std::tuple<uint16_t, uint16_t>> free_space; // block start, count

uint32_t launcher_offset = ~0;

uint8_t buffer[PAGE_SIZE];
uint8_t verify_buffer[PAGE_SIZE];

State		state = stFlashFile;

FIL file;

Dialog dialog;

void scan_flash();
uint32_t flash_from_sd_to_qspi_flash(const char *filename);

// metadata

bool parse_flash_metadata(uint32_t offset, BlitGameMetadata &metadata) {

  BlitGameHeader header;

  if(qspi_read_buffer(offset, reinterpret_cast<uint8_t *>(&header), sizeof(header)) != QSPI_OK)
    return false;

  offset += header.end - 0x90000000;

  // out of bounds
  if(offset >= 0x2000000)
    return false;

  uint8_t buf[10];
  if(qspi_read_buffer(offset, buf, 10) != QSPI_OK)
    return false;

  if(memcmp(buf, "BLITMETA", 8) != 0)
    return false;

  const auto metadata_len = sizeof(RawMetadata);
  uint8_t metadata_buf[metadata_len];
  if(qspi_read_buffer(offset + 10, metadata_buf, metadata_len) != QSPI_OK) {
    return false;
  }

  parse_metadata(reinterpret_cast<char *>(metadata_buf), metadata_len, metadata, false);

  return true;
}

bool parse_file_metadata(const std::string &filename, BlitGameMetadata &metadata) {
  FIL fh;
  f_open(&fh, filename.c_str(), FA_READ);

  BlitGameHeader header;
  UINT bytes_read;
  f_read(&fh, &header, sizeof(header), &bytes_read);

  // skip relocation data
  int off = 0;
  if(header.magic == 0x4F4C4552 /* RELO */) {
    f_lseek(&fh, 4);
    uint32_t num_relocs;
    f_read(&fh, (void *)&num_relocs, 4, &bytes_read);

    off = num_relocs * 4 + 8;
    f_lseek(&fh, off);

    // re-read header
    f_read(&fh, &header, sizeof(header), &bytes_read);
  }

  if(header.magic == blit_game_magic) {
    uint8_t buf[10];
    f_lseek(&fh, (header.end - 0x90000000) + off);
    auto res = f_read(&fh, buf, 10, &bytes_read);

    if(bytes_read == 10 && memcmp(buf, "BLITMETA", 8) == 0) {
      // don't bother reading the whole thing since we don't want the images
      const auto metadata_len = sizeof(RawMetadata);

      uint8_t metadata_buf[metadata_len];
      f_read(&fh, metadata_buf, metadata_len, &bytes_read);

      parse_metadata(reinterpret_cast<char *>(metadata_buf), metadata_len, metadata, false);

      f_close(&fh);
      return true;
    }
  }

  f_close(&fh);
  return false;
}

int calc_num_blocks(uint32_t size) {
  return (size - 1) / qspi_flash_sector_size + 1;
}

void erase_qspi_flash(uint32_t start_sector, uint32_t size_bytes) {
  uint32_t sector_count = calc_num_blocks(size_bytes);

  progress.show("Erasing flash sectors...", sector_count);

  for(uint32_t sector = 0; sector < sector_count; sector++) {
    qspi_sector_erase((start_sector + sector) * qspi_flash_sector_size);

    progress.update(sector);
  }

  progress.hide();
}

// returns true is there is a valid header here
bool read_flash_game_header(uint32_t offset, BlitGameHeader &header) {
  if(qspi_read_buffer(offset, reinterpret_cast<uint8_t *>(&header), sizeof(header)) != QSPI_OK)
    return false;

  if(header.magic != blit_game_magic)
    return false;

  // make sure end/size is sensible
  if(header.end <= qspi_flash_address)
    return false;

  return true;
}

void scan_flash() {
  game_list.clear();
  free_space.clear();

  BlitGameMetadata meta;
  GameInfo game;
  uint32_t free_start = 0xFFFFFFFF;

  for(uint32_t offset = 0; offset < qspi_flash_size;) {
    BlitGameHeader header;

    if(!read_flash_game_header(offset, header)) {
      if(free_start == 0xFFFFFFFF)
        free_start = offset;

      offset += qspi_flash_sector_size;
      continue;
    }

    // add free space to list
    if(free_start != 0xFFFFFFFF) {
      auto start_block = free_start / qspi_flash_sector_size;
      auto end_block = offset / qspi_flash_sector_size;

      free_space.emplace_back(start_block, end_block - start_block);

      free_start = 0xFFFFFFFF;
    }

    game.offset = offset;
    game.size = header.end - qspi_flash_address;

    // check for valid metadata
    if(parse_flash_metadata(offset, meta)) {
      game.title = meta.title;
      game.author = meta.author;
      game.size += meta.length + 10;
      game.checksum = meta.crc32;

      if(meta.title == "Launcher")
        launcher_offset = offset;
    }

    game_list.push_back(game);

    offset += calc_num_blocks(game.size) * qspi_flash_sector_size;
  }

  // final free
  if(free_start != 0xFFFFFFFF) {
    auto start_block = free_start / qspi_flash_sector_size;
    auto end_block = qspi_flash_size / qspi_flash_sector_size;

    free_space.emplace_back(start_block, end_block - start_block);
  }
}

void launch_game(uint32_t address) {
  blit_switch_execution(address, false);
}

bool launch_game_from_sd(const char *path) {

  if(strncmp(path, "flash:/", 7) == 0) {
    blit_switch_execution(atoi(path + 7) * qspi_flash_sector_size, true);
    return true;
  }

  if(is_qspi_memorymapped()) {
    qspi_disable_memorymapped_mode();
    blit_disable_user_code(); // assume user running
  }

  uint32_t offset = 0xFFFFFFFF;

  BlitGameMetadata meta;

  if(parse_file_metadata(path, meta)) {
    for(auto &flash_game : game_list) {
      // if a game with the same name/crc is already installed, launch that one instead of flashing it again
      if(flash_game.checksum == meta.crc32 && flash_game.title == meta.title) {
        offset = flash_game.offset;
        break;
      }
    }
  }

  if(offset == 0xFFFFFFFF)
    offset = flash_from_sd_to_qspi_flash(path);

  if(offset != 0xFFFFFFFF) {
    blit_switch_execution(offset, true);
    return true;
  }

  blit_enable_user_code();

  return false;
}

static void start_launcher() {
  if(launcher_offset != 0xFFFFFFFF)
    launch_game(launcher_offset);
  // no launcher flashed, try to find one on the SD card
  else if(::file_exists("launcher.blit"))
    launch_game_from_sd("launcher.blit");
}

void init() {
  api.launch = launch_game_from_sd;

  set_screen_mode(ScreenMode::hires);
  screen.clear();

  scan_flash();

  // register PROG
  g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'P', 'R', 'O', 'G'>::value, &flashLoader);

  // register SAVE
  g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'S', 'A', 'V', 'E'>::value, &flashLoader);

  // register LS
  g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'_', '_', 'L', 'S'>::value, &flashLoader);

  g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'E', 'R', 'S', 'E'>::value, &cdc_erase_handler);

  // auto-launch
  if(persist.reset_target == prtGame)
    launch_game(persist.last_game_offset);
  // error reset handling
  else if(persist.reset_error) {
    dialog.show("Oops!", "Restart game?", [](bool yes){

      if(yes)
        launch_game(persist.last_game_offset);
      else if(launcher_offset != 0xFFFFFFFF)
        start_launcher();

      persist.reset_error = false;
    });
  } else 
    start_launcher();
}

void render(uint32_t time) {

  if(launcher_offset == 0xFFFFFFFF) {
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.pen = Pen(255, 255, 255);
    screen.text(
      "No launcher found!\n\nFlash one with 32blit flash\n or place launcher.blit on your SD card.",
      minimal_font, Point(screen.bounds.w / 2, screen.bounds.h / 2), true, TextAlign::center_center
    );
  }

  progress.draw();
  dialog.draw();
}

void update(uint32_t time) {
  if(dialog.update())
    return;
}

// returns address to flash file to
uint32_t get_flash_offset_for_file(uint32_t file_size) {

  int file_blocks = calc_num_blocks(file_size);

  for(auto space : free_space) {
    if(std::get<1>(space) >= file_blocks)
      return std::get<0>(space) * qspi_flash_sector_size;
  }

  // TODO: handle flash full
  return 0;
}

// Flash(): Flash a file from the SDCard to external flash
uint32_t flash_from_sd_to_qspi_flash(const char *filename)
{
  BlitGameMetadata meta;
  bool has_meta = parse_file_metadata(filename, meta);

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

  // check for prepended relocation info
  char buf[4];
  f_read(&file, buf, 4, &bytes_read);
  std::vector<uint32_t> relocation_offsets;
  size_t cur_reloc = 0;
  bool has_relocs = false;

  if(memcmp(buf, "RELO", 4) == 0) {
    uint32_t num_relocs;
    f_read(&file, (void *)&num_relocs, 4, &bytes_read);
    relocation_offsets.reserve(num_relocs);

    for(auto i = 0u; i < num_relocs; i++) {
      uint32_t reloc_offset;
      f_read(&file, (void *)&reloc_offset, 4, &bytes_read);

      relocation_offsets.push_back(reloc_offset - 0x90000000);
    }

    bytes_total -= num_relocs * 4 + 8; // size of relocation data
    has_relocs = true;
  } else {
    f_lseek(&file, 0);
  }

  // check header
  auto off = f_tell(&file);
  BlitGameHeader header;
  if(f_read(&file, (void *)&header, sizeof(header), &bytes_read) != FR_OK) {
    f_close(&file);
    return false;
  }
  f_lseek(&file, off);

  uint32_t flash_offset = 0xFFFFFFFF;

  if(!has_relocs)
    flash_offset = 0;
  // check for other versions of the same thing
  else if(has_meta) {
    for(auto &game : game_list) {
      if(game.title == meta.title && game.author == meta.author) {
        if(calc_num_blocks(game.size) <= calc_num_blocks(bytes_total)) {
          flash_offset = game.offset;
          break;
        } else {
          // new version is bigger, erase old one
          erase_qspi_flash(game.offset / qspi_flash_sector_size, game.size);
        }
      }
    }
  }

  if(flash_offset == 0xFFFFFFFF)
    flash_offset = get_flash_offset_for_file(bytes_total);

  // erase the sectors needed to write the image
  erase_qspi_flash(flash_offset / qspi_flash_sector_size, bytes_total);

  progress.show("Copying from SD card to flash...", bytes_total);

  const int buffer_size = 4096;
  uint8_t buffer[buffer_size];
  uint8_t verify_buffer[buffer_size];

  while(bytes_flashed < bytes_total) {
    // limited ram so a bit at a time
    res = f_read(&file, (void *)buffer, buffer_size, &bytes_read);

    if(res != FR_OK)
      break;

    // relocation patching
    if(cur_reloc < relocation_offsets.size()) {
      for(auto off = offset; off < offset + bytes_read; off += 4) {
        if(off == relocation_offsets[cur_reloc]) {
          *(uint32_t *)(buffer + off - offset) += flash_offset;
          cur_reloc++;
        }
      }
    }

    if(qspi_write_buffer(offset + flash_offset, buffer, bytes_read) != QSPI_OK)
      break;

    if(qspi_read_buffer(offset + flash_offset, verify_buffer, bytes_read) != QSPI_OK)
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


void cdc_flash_list() {
  bool mapped = is_qspi_memorymapped();

  if(mapped)
    qspi_disable_memorymapped_mode();

  // scan through flash and send offset, size and metadata
  for(uint32_t offset = 0; offset < qspi_flash_size;) {
    BlitGameHeader header;

    if(!read_flash_game_header(offset, header)) {
      offset += qspi_flash_sector_size;
      continue;
    }

    uint32_t size = header.end - qspi_flash_address;

    // metadata header
    uint8_t buf[10];
    if(qspi_read_buffer(offset + size, buf, 10) != QSPI_OK)
      break;

    while(CDC_Transmit_HS((uint8_t *)&offset, 4) == USBD_BUSY){}
    while(CDC_Transmit_HS((uint8_t *)&size, 4) == USBD_BUSY){}

    uint16_t metadata_len = 0;

    if(memcmp(buf, "BLITMETA", 8) == 0)
      metadata_len = *reinterpret_cast<uint16_t *>(buf + 8);

    while(CDC_Transmit_HS((uint8_t *)"BLITMETA", 8) == USBD_BUSY){}
    while(CDC_Transmit_HS((uint8_t *)&metadata_len, 2) == USBD_BUSY){}

    // send metadata
    uint32_t metadata_offset = offset + size + 10;
    while(metadata_len) {
      int chunk_size = std::min(256, (int)metadata_len);
      uint8_t metadata_buf[256];

      if(qspi_read_buffer(metadata_offset, metadata_buf, chunk_size) != QSPI_OK)
        break;

      while(CDC_Transmit_HS(metadata_buf, chunk_size) == USBD_BUSY){}

      metadata_offset += chunk_size;
      metadata_len -= chunk_size;
    }

    offset += calc_num_blocks(size) * qspi_flash_sector_size;
  }

  // end marker
  uint32_t end = 0xFFFFFFFF;
  while(CDC_Transmit_HS((uint8_t *)&end, 4) == USBD_BUSY){}

  if(mapped)
    qspi_enable_memorymapped_mode();
}

// erase command handler
CDCCommandHandler::StreamResult CDCEraseHandler::StreamData(CDCDataStream &dataStream) {
  uint32_t offset;
  if(!dataStream.Get(offset))
    return srNeedData;

  // reject unaligned
  if(offset & (qspi_flash_sector_size - 1))
    return srFinish;

  bool flash_mapped = is_qspi_memorymapped();
  if(flash_mapped) {
    blit_disable_user_code();
    qspi_disable_memorymapped_mode();
  }

  // attempt to get size, falling back to a single sector
  int erase_size = 1;
  BlitGameHeader header;
  if(read_flash_game_header(offset, header))
    erase_size = calc_num_blocks(header.end - qspi_flash_address); // TODO: this does not include metadata, may result in some leftover junk

  erase_qspi_flash(offset / qspi_flash_sector_size, erase_size * qspi_flash_sector_size);

  // rescan
  scan_flash();

  if(flash_mapped) {
    qspi_enable_memorymapped_mode();
    blit_enable_user_code();
  }

  return srFinish;
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

      flash_mapped = is_qspi_memorymapped();
      if(flash_mapped) {
        blit_disable_user_code();
        qspi_disable_memorymapped_mode();
      }
    break;

    case CDCCommandHandler::CDCFourCCMake<'S', 'A', 'V', 'E'>::value:
      state = stSaveFile;
      m_parseState = stFilename;
      m_uParseIndex = 0;
      blit_disable_user_code();
    break;

    case CDCCommandHandler::CDCFourCCMake<'_', '_', 'L', 'S'>::value:
      bNeedStream = false;
      cdc_flash_list();
    break;

  }

  return bNeedStream;
}


// FlashData() Flash data to the QSPI flash
// Note: currently qspi_write_buffer only works for sizes of 256 max
bool FlashData(uint32_t start, uint32_t uOffset, uint8_t *pBuffer, uint32_t uLen)
{
  bool bResult = false;
  if(QSPI_OK == qspi_write_buffer(start + uOffset, pBuffer, uLen))
  {
    if(QSPI_OK == qspi_read_buffer(start + uOffset, verify_buffer, uLen))
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
                    m_parseState = stRelocs;
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

      case stRelocs: {
        uint32_t word;
        if(m_uParseIndex > 1 && m_uParseIndex == num_relocs + 2) {
          cur_reloc = 0;
          m_parseState = stData;
          m_uParseIndex = 0;
        } else {
          while(result == srContinue && dataStream.Get(word)) {
            if(m_uParseIndex == 0 && word != 0x4F4C4552 /*RELO*/) {
              printf("Missing relocation header\n");
              result = srError;
            } else if(m_uParseIndex == 1) {
              num_relocs = word;
              m_uFilelen -= num_relocs * 4 + 8;
              relocation_offsets.reserve(num_relocs);
            } else if(m_uParseIndex)
              relocation_offsets.push_back(word - 0x90000000);

            m_uParseIndex++;

            // done
            if(m_uParseIndex == num_relocs + 2)
              break;
          }
        }
        break;
      }

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

                    state = stFlashFile;
                    if(result != srError)
                      result = srFinish;

                    progress.hide();
                    blit_enable_user_code();
                  }
                break;

                case stFlashCDC:
                {
                  uint32_t uPage = (m_uParseIndex / PAGE_SIZE);
                  // first page, check header
                  if(uPage == 0) {
                    flash_start_offset = get_flash_offset_for_file(m_uFilelen);

                    // erase
                    erase_qspi_flash(flash_start_offset / qspi_flash_sector_size, m_uFilelen);
                    progress.show("Saving " + std::string(m_sFilename) +  " to flash...", m_uFilelen);
                  }

                  // relocation patching
                  if(cur_reloc < relocation_offsets.size()) {
                    auto offset = uPage * PAGE_SIZE;

                    for(auto off = offset; off < offset + uWriteLen; off += 4) {
                      if(off == relocation_offsets[cur_reloc]) {
                        *(uint32_t *)(buffer + off - offset) += flash_start_offset;
                        cur_reloc++;
                      }
                    }
                  }

                  // save data
                  if(!FlashData(flash_start_offset, uPage*PAGE_SIZE, buffer, uWriteLen))
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

                      // clean up old version(s)
                      BlitGameMetadata meta;
                      if(parse_flash_metadata(flash_start_offset, meta)) {
                        for(auto &game : game_list) {
                          if(game.title == meta.title && game.author == meta.author && game.offset != flash_start_offset)
                            erase_qspi_flash(game.offset / qspi_flash_sector_size, game.size);
                        }
                      }

                      blit_switch_execution(flash_start_offset, true);
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

    if(flash_mapped) {
      qspi_enable_memorymapped_mode();
      flash_mapped = false;
    }

    blit_enable_user_code();
  }

  return result;
}


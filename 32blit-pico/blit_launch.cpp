#include <cstring>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/multicore.h"

#ifdef PICO_RP2350
#include "hardware/structs/qmi.h"
#endif

#include "blit_launch.hpp"
#include "file.hpp"

#include "engine/engine.hpp"
#include "engine/file.hpp"

#ifdef PICO_RP2350
#define DEVICE_ID BlitDevice::RP2350
#define FLASH_BASE XIP_NOCACHE_NOALLOC_NOTRANSLATE_BASE
#else
#define DEVICE_ID BlitDevice::RP2040
#define FLASH_BASE XIP_NOCACHE_NOALLOC_BASE
#endif

// code related to blit files and launching

extern int (*do_tick)(uint32_t time);

void disable_user_code();

extern bool core1_started;

static uint32_t requested_launch_offset = 0;
static uint32_t current_game_offset = 0;

static uint32_t get_installed_file_size(uint32_t offset) {
  auto header = (BlitGameHeader *)(FLASH_BASE + offset);

  // check header magic + device
  if(header->magic != blit_game_magic || header->device_id != DEVICE_ID)
    return 0;

  auto size = header->end;

  // check metadata
  auto meta_offset = offset + size;
  if(memcmp((char *)(FLASH_BASE + meta_offset), "BLITMETA", 8) == 0) {
    // add metadata size
    size += *(uint16_t *)(FLASH_BASE + meta_offset + 8) + 10;
  }

  return size;
}

static uint32_t calc_num_blocks(uint32_t size) {
  return (size - 1) / game_block_size + 1;
}

static uint32_t find_flash_offset(uint32_t requested_size) {
  uint32_t free_off = 0; // 0 is invalid as that's where the loader is

  // FIXME: avoid flash storage

  for(uint32_t off = 256 * 1024; off < PICO_FLASH_SIZE_BYTES;) {
    auto size = get_installed_file_size(off);

    if(!size) {
      // empty, store offset
      if(!free_off)
        free_off = off;

      off += game_block_size;
      continue;
    }

    if(free_off) {
      // end of free space, check if large enough
      auto found_space = off - free_off;

      if(found_space >= requested_size)
        return free_off;

      free_off = 0;
    }

    // skip to end
    off += calc_num_blocks(size) * game_block_size;
  }

  // last chance
  if(free_off && PICO_FLASH_SIZE_BYTES - free_off >= requested_size)
    return free_off;

  return 0;
}

static bool read_file_metadata(void *file, RawMetadata &meta, RawTypeMetadata &type_meta) {
  // read header and check magic
  BlitGameHeader header;
  if(read_file(file, 0, sizeof(header), (char *)&header) != sizeof(header))
    return false;

  if(header.magic != blit_game_magic)
    return false;

  // read and check metadata header
  auto meta_offset = header.end;

  char meta_header[10];
  if(read_file(file, meta_offset, 10, meta_header) != 10)
    return false;

  if(memcmp(meta_header, "BLITMETA", 8) != 0)
    return false;

  // read the reset of the metadata header
  if(read_file(file, meta_offset + 10, sizeof(RawMetadata), (char *)&meta) != sizeof(RawMetadata))
    return false;

  // check for type data
  meta_offset += 10 + sizeof(RawMetadata);
  if(read_file(file, meta_offset, 8, meta_header) != 8)
    return false;

  if(memcmp(meta_header, "BLITTYPE", 8) == 0) {
    if(read_file(file, meta_offset + 8, sizeof(RawTypeMetadata), (char *)&type_meta) != sizeof(RawTypeMetadata))
      return false;
  }

  return true;
}

// 32blit API

RawMetadata *get_running_game_metadata() {
#ifdef BUILD_LOADER
  if(!current_game_offset)
    return nullptr;

  auto game_ptr = reinterpret_cast<uint8_t *>(FLASH_BASE + current_game_offset);

  auto header = reinterpret_cast<BlitGameHeader *>(game_ptr);

  if(header->magic == blit_game_magic) {
    auto end_ptr = game_ptr + header->end;
    if(memcmp(end_ptr, "BLITMETA", 8) == 0)
      return reinterpret_cast<RawMetadata *>(end_ptr + 10);
  }

#endif
  return nullptr;
}

bool launch_file(const char *path) {
  uint32_t flash_offset;

  if(strncmp(path, "flash:/", 7) == 0) // from flash
    flash_offset = atoi(path + 7) * game_block_size;
  else {
    // from storage
    // TODO: check if already in flash

    auto file = open_file(path, blit::OpenMode::read);

    if(!file)
      return false;

    BlitWriter writer;

    uint32_t file_offset = 0;
    uint32_t len = get_file_length(file);

    writer.init(len);

    // read in small chunks
    uint8_t buf[FLASH_PAGE_SIZE];

    while(file_offset < len) {
      auto bytes_read = read_file(file, file_offset, FLASH_PAGE_SIZE, (char *)buf);
      if(bytes_read <= 0)
        break;

      if(!writer.write(buf, bytes_read))
        break;

      file_offset += bytes_read;
    }

    close_file(file);

    // didn't write everything, fail launch
    if(writer.get_remaining() > 0)
      return false;

    flash_offset = writer.get_flash_offset();
  }

  auto header = (BlitGameHeader *)(FLASH_BASE + flash_offset);
  // check header magic + device
  if(header->magic != blit_game_magic || header->device_id != DEVICE_ID)
    return false;

  if(!header->init || !header->render || !header->tick)
    return false;

  requested_launch_offset = flash_offset;
  return true;
}

blit::CanLaunchResult can_launch(const char *path) {
#ifdef BUILD_LOADER
  if(strncmp(path, "flash:/", 7) == 0) {
    // assume anything flashed is compatible for now
    return blit::CanLaunchResult::Success;
  }

  // get the extension
  std::string_view sv(path);
  auto last_dot = sv.find_last_of('.');
  auto ext = last_dot == std::string::npos ? "" : std::string(sv.substr(last_dot + 1));
  for(auto &c : ext)
    c = tolower(c);

  if(ext == "blit") {
    BlitGameHeader header;
    auto file = open_file(path, blit::OpenMode::read);

    if(!file)
      return blit::CanLaunchResult::InvalidFile;

    auto bytes_read = read_file(file, 0, sizeof(header), (char *)&header);

    if(bytes_read == sizeof(header) && header.magic == blit_game_magic && header.device_id == DEVICE_ID) {
      close_file(file);
      return blit::CanLaunchResult::Success;
    }

    close_file(file);
    return blit::CanLaunchResult::IncompatibleBlit;
  }
#endif

  return blit::CanLaunchResult::UnknownType;
}

void delayed_launch() {
  if(!requested_launch_offset)
    return;

  auto header = (BlitGameHeader *)(FLASH_BASE + requested_launch_offset);

#ifdef PICO_RP2350
  uint32_t header_offset = *(uint32_t *)(FLASH_BASE + requested_launch_offset + sizeof(BlitGameHeader));
  if(header_offset != requested_launch_offset) {
    // setup translation
    uint32_t size = 4 * 1024 * 1024; // TODO: use (rounded) blit size
    qmi_hw->atrans[1] = (size >> 12) << QMI_ATRANS1_SIZE_LSB
                      | (requested_launch_offset >> 12) << QMI_ATRANS1_BASE_LSB;

    // invalidate cache
    for(uint32_t off = 0; off < size; off += 8)
      *(uint8_t *)(XIP_MAINTENANCE_BASE + 4 * 1024 * 1024 + off + 2/*invalidate by addr*/) = 0;

    // FIXME: handle previous blit also using translation on failure
  }
#endif

  // save in case launch fails
  uint32_t last_game_offset = current_game_offset;

  current_game_offset = requested_launch_offset;
  requested_launch_offset = 0;

  if(!header->init(0)) {
    blit::debugf("failed to init game!\n");
    current_game_offset = last_game_offset;
    return;
  }

  blit::render = header->render;
  do_tick = header->tick;
}

void list_installed_games(std::function<void(const uint8_t *, uint32_t, uint32_t)> callback) {
  for(uint32_t off = 0; off < PICO_FLASH_SIZE_BYTES;) {
    auto size = get_installed_file_size(off);

    if(!size) {
      off += game_block_size;
      continue;
    }

    callback((const uint8_t *)(FLASH_BASE + off), off / game_block_size, size);

    off += calc_num_blocks(size) * game_block_size;
  }
}

void erase_game(uint32_t offset) {
#ifdef BUILD_LOADER
  // check alignment
  if(offset & (game_block_size - 1))
    return;

  // check in bounds
  // TODO: prevent erasing fs if flash storage is used?
  if(offset >= PICO_FLASH_SIZE_BYTES)
    return;

  auto size = get_installed_file_size(offset);

  // fall back to one block if size unknown
  auto num_blocks = size == 0 ? 1 : calc_num_blocks(size);

  // do erase
  auto status = save_and_disable_interrupts();

  if(core1_started)
    multicore_lockout_start_blocking(); // pause core1

  // the real erase size is smaller than the one baked into the API...
  static_assert(game_block_size % FLASH_SECTOR_SIZE == 0);
  flash_range_erase(offset, num_blocks * game_block_size);

  if(core1_started)
    multicore_lockout_end_blocking(); // resume core1

  restore_interrupts(status);
#endif
}

// .blit file writer
void BlitWriter::init(uint32_t file_len) {
  this->file_len = file_len;
  file_offset = flash_offset = 0;
}

bool BlitWriter::write(const uint8_t *buf, uint32_t len) {
  if(!flash_offset) {
    if(!prepare_write(buf))
      return false;
  }

  if(file_offset >= file_len)
    return false;

  // write page
  auto status = save_and_disable_interrupts();

  if(core1_started)
    multicore_lockout_start_blocking(); // pause core1

  // assuming len <= page size and buf size == page size
  flash_range_program(flash_offset + file_offset, buf, FLASH_PAGE_SIZE);

  if(core1_started)
    multicore_lockout_end_blocking(); // resume core1

  restore_interrupts(status);

  file_offset += len;

  return true;
}

uint32_t BlitWriter::get_remaining() const {
  return file_len - file_offset;
}

uint32_t BlitWriter::get_flash_offset() const {
  return flash_offset;
}

bool BlitWriter::prepare_write(const uint8_t *buf) {
  auto header = (BlitGameHeader *)buf;

  if(header->magic != blit_game_magic || header->device_id != DEVICE_ID) {
    blit::debugf("Invalid blit header!");
    return false;
  }

  // currently non-relocatable, so base address is stored after header
  flash_offset = *(uint32_t *)(buf + sizeof(BlitGameHeader));
  flash_offset &= 0xFFFFFF;

#ifdef PICO_RP2350
  if(flash_offset == 4 * 1024 * 1024) {
    // we can use address translation for this, so flash in any free space
    flash_offset = find_flash_offset(file_len);
  }
#endif

  printf("PROG: flash off %lu\n", flash_offset);

  disable_user_code();

  // erase flash
  auto status = save_and_disable_interrupts();

  if(core1_started)
    multicore_lockout_start_blocking(); // pause core1

  auto erase_size = ((file_len - 1) / FLASH_SECTOR_SIZE) + 1;
  flash_range_erase(flash_offset, erase_size * FLASH_SECTOR_SIZE);

  if(core1_started)
    multicore_lockout_end_blocking(); // resume core1

  restore_interrupts(status);

  return true;
}

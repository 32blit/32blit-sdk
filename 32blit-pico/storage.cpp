// raw storage interface, currently flash
#include "storage.hpp"

#include <cstring>

#include "hardware/flash.h"
#include "hardware/sync.h"

static const uint32_t storage_size = PICO_FLASH_SIZE_BYTES / 4;
static const uint32_t storage_offset = PICO_FLASH_SIZE_BYTES - storage_size;

void get_storage_size(uint16_t &block_size, uint32_t &num_blocks) {
  block_size = FLASH_SECTOR_SIZE;
  num_blocks = storage_size / FLASH_SECTOR_SIZE;
}

int32_t storage_read(uint32_t sector, uint32_t offset, void *buffer, uint32_t size_bytes) {
  memcpy(buffer, (uint8_t *)XIP_NOCACHE_NOALLOC_BASE + storage_offset + sector * FLASH_SECTOR_SIZE + offset, size_bytes);

  return size_bytes;
}

int32_t storage_write(uint32_t sector, uint32_t offset, const uint8_t *buffer, uint32_t size_bytes) {
  auto status = save_and_disable_interrupts();

  if(offset == 0)
    flash_range_erase(storage_offset + sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);

  flash_range_program(storage_offset + sector * FLASH_SECTOR_SIZE + offset, buffer, size_bytes);

  restore_interrupts(status);

  return size_bytes;
}

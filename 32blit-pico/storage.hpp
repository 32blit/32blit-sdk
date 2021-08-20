#pragma once
#include <cstdint>

void get_storage_size(uint16_t &block_size, uint32_t &num_blocks);

int32_t storage_read(uint32_t sector, uint32_t offset, void *buffer, uint32_t size_bytes);
int32_t storage_write(uint32_t sector, uint32_t offset, const uint8_t *buffer, uint32_t size_bytes);

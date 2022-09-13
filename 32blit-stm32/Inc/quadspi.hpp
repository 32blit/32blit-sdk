#pragma once
#include <cstdint>

constexpr uint32_t qspi_flash_sector_size = 64 * 1024;
constexpr uint32_t qspi_flash_size = 32768 * 1024;
constexpr uint32_t qspi_flash_address = 0x90000000;

// resevered space for temp/cached files
constexpr uint32_t qspi_tmp_reserved = 4 * 1024 * 1024;

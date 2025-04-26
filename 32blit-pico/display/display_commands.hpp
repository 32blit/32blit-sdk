#pragma once
#include <cstdint>

// standard display command set
enum MIPIDCS {
  Nop              = 0x00,
  SoftReset        = 0x01,
  GetAddressMode   = 0x0B,
  GetPixelFormat   = 0x0C,
  EnterSleepMode   = 0x10,
  ExitSleepMode    = 0x11,
  ExitInvertMode   = 0x20,
  EnterInvertMode  = 0x21,
  DisplayOff       = 0x28,
  DisplayOn        = 0x29,
  SetColumnAddress = 0x2A,
  SetRowAddress    = 0x2B,
  WriteMemoryStart = 0x2C,
  SetTearOff       = 0x34,
  SetTearOn        = 0x35,
  SetAddressMode   = 0x36,
  SetPixelFormat   = 0x3A,
  SetTearScanline  = 0x44,
};


enum MADCTL : uint8_t {
  // writing to internal memory
  ROW_ORDER   = 0b10000000,  // MY / y flip
  COL_ORDER   = 0b01000000,  // MX / x flip
  SWAP_XY     = 0b00100000,  // AKA "MV"

  // scanning out from internal memory
  SCAN_ORDER  = 0b00010000,
  RGB         = 0b00001000,
  HORIZ_ORDER = 0b00000100
};

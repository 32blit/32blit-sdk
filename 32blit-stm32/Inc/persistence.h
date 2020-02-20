#pragma once

/*
In combination with a reserved section in volatile memory this struct provies 1k
of "persistent" storage in volatile RAM.

This storage is reserved for 32blit firmware level use, and will survive resets,
switches into bootloader mode and jumps between internal/external flash.

This storage will not survive a loss of power.
*/
struct Persist {
  uint32_t magic_word;
  float volume;
  float backlight;
  uint32_t selected_menu_item;
  uint32_t reset_target;
};

extern Persist persist;

// This magic word lets us know our persistent values are valid
// and not just random uninitialised memory.
constexpr uint32_t persistence_magic_word = 0x03281170;
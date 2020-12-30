/*
In combination with a reserved section in volatile memory this struct provies 1k
of "persistent" storage in volatile RAM.

This storage is reserved for 32blit firmware level use, and will survive resets,
switches into bootloader mode and jumps between internal/external flash.

This storage will not survive a loss of power.
*/
#pragma once

typedef enum {prtFirmware, prtGame} PersistResetTarget;

struct Persist {
  uint32_t magic_word;

  // set to true if the volume is muted
  bool is_muted;

  // the current volume level -- muting does not affect this
  float volume;

  // the current backlight level
  float backlight;
  uint32_t selected_menu_item;

  PersistResetTarget reset_target;
  bool reset_error; // last reset was caused by an error
  uint32_t last_game_offset;

  char launch_path[256];
};

extern Persist persist;

// This magic word lets us know our persistent values are valid
// and not just random uninitialised memory.
constexpr uint32_t persistence_magic_word = 0x03281170;

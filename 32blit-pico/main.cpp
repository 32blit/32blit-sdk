#include <cstdio>
#include <cstring>

#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "hardware/structs/rosc.h"
#include "hardware/vreg.h"
#include "hardware/timer.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"

#include "audio.hpp"
#include "binary_info.hpp"
#include "config.h"
#include "display.hpp"
#include "file.hpp"
#include "input.hpp"
#include "led.hpp"
#include "multiplayer.hpp"
#include "storage.hpp"
#include "usb.hpp"

#include "executable.hpp"

#include "engine/api_private.hpp"

using namespace blit;

const unsigned int game_block_size = 64 * 1024; // this is the 32blit's flash erase size, some parts of the API depend on this...

static blit::AudioChannel channels[CHANNEL_COUNT];

static int (*do_tick)(uint32_t time) = blit::tick;

static uint32_t requested_launch_offset = 0;

// override terminate handler to save ~20-30k
namespace __cxxabiv1 {
  std::terminate_handler __terminate_handler = std::abort;
}

static uint32_t now() {
  return to_ms_since_boot(get_absolute_time());
}

static uint32_t random() {
	return get_rand_32();
}

static void debug(const char *message) {
  auto p = message;
  while(*p)
    putchar(*p++);
}

static uint32_t get_us_timer() {
  return to_us_since_boot(get_absolute_time());
}

static uint32_t get_max_us_timer() {
  return 0xFFFFFFFF; // it's a 64bit timer...
}

const char *get_launch_path()  {
  return nullptr;
}

static GameMetadata get_metadata() {
  GameMetadata ret;

  // parse binary info
  extern binary_info_t *__binary_info_start, *__binary_info_end;

  for(auto tag_ptr = &__binary_info_start; tag_ptr != &__binary_info_end ; tag_ptr++) {
    if((*tag_ptr)->type != BINARY_INFO_TYPE_ID_AND_STRING)
      continue;

    auto id_str_tag = (binary_info_id_and_string_t *)*tag_ptr;

    if((*tag_ptr)->tag == BINARY_INFO_TAG_RASPBERRY_PI) {
      switch(id_str_tag->id) {
        case BINARY_INFO_ID_RP_PROGRAM_NAME:
          ret.title = id_str_tag->value;
          break;
        case BINARY_INFO_ID_RP_PROGRAM_VERSION_STRING:
          ret.version = id_str_tag->value;
          break;
        case BINARY_INFO_ID_RP_PROGRAM_URL:
          ret.url = id_str_tag->value;
          break;
        case BINARY_INFO_ID_RP_PROGRAM_DESCRIPTION:
          ret.description = id_str_tag->value;
          break;
      }
    } else if((*tag_ptr)->tag == BINARY_INFO_TAG_32BLIT) {
      switch(id_str_tag->id) {
        case BINARY_INFO_ID_32BLIT_AUTHOR:
          ret.author = id_str_tag->value;
          break;
        case BINARY_INFO_ID_32BLIT_CATEGORY:
          ret.category = id_str_tag->value;
          break;
      }
    }

  }
  return ret;
}

static bool launch(const char *path) {
  if(strncmp(path, "flash:/", 7) == 0) {
    uint32_t offset = atoi(path + 7) * game_block_size;

    auto header = (BlitGameHeader *)(XIP_NOCACHE_NOALLOC_BASE + offset);
    // check header magic + device
    if(header->magic != blit_game_magic || header->device_id != BlitDevice::RP2040)
      return false;

    if(!header->init || !header->render || !header->tick)
      return false;

    requested_launch_offset = offset;
    return true;
  }

  return false;
}

static blit::CanLaunchResult can_launch(const char *path) {
  if(strncmp(path, "flash:/", 7) == 0) {
    // assume anything flashed is compatible for now
    return blit::CanLaunchResult::Success;
  }

  return blit::CanLaunchResult::UnknownType;
}

static void delayed_launch() {
  auto header = (BlitGameHeader *)(XIP_NOCACHE_NOALLOC_BASE + requested_launch_offset);
  requested_launch_offset = 0;

  if(!header->init(0)) {
    debugf("failed to init game!\n");
    return;
  }

  blit::render = header->render;
  do_tick = header->tick;
}

static void list_installed_games(std::function<void(const uint8_t *, uint32_t, uint32_t)> callback) {
  for(uint32_t off = 0; off < PICO_FLASH_SIZE_BYTES;) {
    auto header = (BlitGameHeader *)(XIP_NOCACHE_NOALLOC_BASE + off);

    // check header magic + device
    if(header->magic != blit_game_magic || header->device_id != BlitDevice::RP2040) {
      off += game_block_size;
      continue;
    }

    auto size = header->end;

    // check metadata
    auto meta_offset = off + size;
    if(memcmp((char *)(XIP_NOCACHE_NOALLOC_BASE + meta_offset), "BLITMETA", 8) != 0) {
      off += ((size - 1) / game_block_size + 1) * game_block_size;
      continue;
    }

    // add metadata size
    size += *(uint16_t *)(XIP_NOCACHE_NOALLOC_BASE + meta_offset + 8) + 10;

    callback((const uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + off), off / game_block_size, size);

    off += ((size - 1) / game_block_size + 1) * game_block_size;
  }
}

static uint8_t *get_screen_data() {
  return screen.data;
}

// blit API
[[gnu::section(".rodata.api_const")]]
static const blit::APIConst blit_api_const {
  blit::api_version_major, blit::api_version_minor,

  ::channels,

  ::set_screen_mode,
  ::set_screen_palette,

  ::now,
  ::random,
  nullptr, // exit
  ::debug,

  ::open_file,
  ::read_file,
  ::write_file,
  ::close_file,
  ::get_file_length,
  ::list_files,
  ::file_exists,
  ::directory_exists,
  ::create_directory,
  ::rename_file,
  ::remove_file,
  ::get_save_path,
  ::is_storage_available,

  nullptr, // enable_us_timer
  ::get_us_timer,
  ::get_max_us_timer,

  nullptr, // decode_jpeg_buffer
  nullptr, // decode_jpeg_file

  ::launch,
  nullptr, // erase_game
  nullptr, // get_type_handler_metadata

  ::get_launch_path,

  ::is_multiplayer_connected,
  ::set_multiplayer_enabled,
  ::send_multiplayer_message,

  nullptr, // flash_to_tmp
  nullptr, // tmp_file_closed

  ::get_metadata,

  ::set_screen_mode_format,

  nullptr, // i2c_send
  nullptr, // i2c_recieve

  nullptr, // set_raw_cdc_enabled
  nullptr, // cdc_write
  nullptr, // cdc_read

  ::list_installed_games,
  ::can_launch,

  ::get_screen_data,
  ::set_framebuffer,
};

[[gnu::section(".bss.api_data")]]
static blit::APIData blit_api_data;

namespace blit {
  const APIConst &api = blit_api_const;
  APIData &api_data = blit_api_data;
}

// user funcs
void init();
void render(uint32_t);
void update(uint32_t);

bool core1_started = false;

#ifdef ENABLE_CORE1
void core1_main() {
  core1_started = true;
  multicore_lockout_victim_init();

  init_display_core1();
  init_audio();

  while(true) {
    update_display_core1();
    update_audio(::now());
    sleep_us(1);
  }
}

#else
static void alarm_callback(uint alarm_num) {
  timer_hw->intr = 1 << alarm_num;
  hardware_alarm_set_target(alarm_num, make_timeout_time_ms(5));

  update_audio(::now());
}
#endif

static void init_i2c() {
  // multiple drivers need i2c, initialise it in one place if needed
#ifdef DEFAULT_I2C_CLOCK
  i2c_init(i2c_default, DEFAULT_I2C_CLOCK);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);

  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
#endif
}

int main() {
#if OVERCLOCK_250
#ifndef PICO_RP2350
  // Apply a modest overvolt, default is 1.10v.
  // this is required for a stable 250MHz on some RP2040s
  vreg_set_voltage(VREG_VOLTAGE_1_20);
  sleep_ms(10);
#endif

  set_sys_clock_khz(250000, false);
#endif

  init_usb();
  stdio_init_all();

  init_i2c();

  init_led();
  init_display();
  init_input();
  init_fs();
#if !defined(ENABLE_CORE1)
  init_audio();
#endif

#if defined(ENABLE_CORE1)
  multicore_launch_core1(core1_main);
#else
  // fallback audio timer if core1 is unavailable / not enabled
  int alarm_num = hardware_alarm_claim_unused(true);
  hardware_alarm_set_callback(alarm_num, alarm_callback);
  hardware_alarm_set_target(alarm_num, make_timeout_time_ms(5));
#ifdef PICO_RP2350
  irq_set_priority(TIMER0_IRQ_0 + alarm_num, PICO_LOWEST_IRQ_PRIORITY);
#else
  irq_set_priority(TIMER_IRQ_0 + alarm_num, PICO_LOWEST_IRQ_PRIORITY);
#endif
#endif

#ifndef BUILD_LOADER
  blit::set_screen_mode(ScreenMode::lores);
#endif

  blit::render = ::render;
  blit::update = ::update;

  // user init
  ::init();

  while(true) {
    auto now = ::now();
    update_display(now);
    update_input();
    int ms_to_next_update = do_tick(::now());
    update_led();
    update_usb();
    update_multiplayer();

    // do requested launch when no user code is running
    if(requested_launch_offset)
      delayed_launch();

    if(ms_to_next_update > 1 && !display_render_needed())
      best_effort_wfe_or_timeout(make_timeout_time_ms(ms_to_next_update - 1));
  }

  return 0;
}

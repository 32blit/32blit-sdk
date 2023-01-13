#include <stdio.h>
#include <random>

#include "hardware/structs/rosc.h"
#include "hardware/vreg.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

#include "audio.hpp"
#include "config.h"
#include "display.hpp"
#include "file.hpp"
#include "input.hpp"
#include "led.hpp"
#include "usb.hpp"

#include "engine/api_private.hpp"

using namespace blit;

static blit::AudioChannel channels[CHANNEL_COUNT];

static uint32_t now() {
  return to_ms_since_boot(get_absolute_time());
}

static uint32_t get_random_seed() {
  uint32_t seed = 0;

  // use the hardware random bit to seed
  for(int i = 0; i < 32; i++) {
    seed <<= 1;
    seed |= rosc_hw->randombit & 1;
    sleep_us(1); // don't read too fast
  }

  return seed;
}

static uint32_t random() {
  static std::mt19937 random_generator(get_random_seed());
  static std::uniform_int_distribution<uint32_t> random_distribution;

	return random_distribution(random_generator);
}

static void debug(const char *message) {
  fputs(message, stdout);

  usb_debug(message);
}

static bool is_storage_available() {
  return true; // TODO: optional storage?
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
    if((*tag_ptr)->type != BINARY_INFO_TYPE_ID_AND_STRING || (*tag_ptr)->tag != BINARY_INFO_TAG_RASPBERRY_PI)
      continue;

    auto id_str_tag = (binary_info_id_and_string_t *)*tag_ptr;

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

  }
  return ret;
}

// user funcs
void init();
void render(uint32_t);
void update(uint32_t);

bool core1_started = false;

void core1_main() {
  core1_started = true;
  multicore_lockout_victim_init();

  init_display_core1();

  while(true) {
    update_display_core1();
    sleep_us(1);
  }
}

int main() {
#if OVERCLOCK_250
  // Apply a modest overvolt, default is 1.10v.
  // this is required for a stable 250MHz on some RP2040s
  vreg_set_voltage(VREG_VOLTAGE_1_20);
  sleep_ms(10);
  set_sys_clock_khz(250000, false);
#endif

  stdio_init_all();

  api.channels = ::channels;

  api.set_screen_mode = ::set_screen_mode;
  api.set_screen_palette = ::set_screen_palette;
  api.set_screen_mode_format = ::set_screen_mode_format;
  api.now = ::now;
  api.random = ::random;
  // api.exit = ::exit;

  // serial debug
  api.debug = ::debug;

  // files
  api.open_file = ::open_file;
  api.read_file = ::read_file;
  api.write_file = ::write_file;
  api.close_file = ::close_file;
  api.get_file_length = ::get_file_length;
  api.list_files = ::list_files;
  api.file_exists = ::file_exists;
  api.directory_exists = ::directory_exists;
  api.create_directory = ::create_directory;
  api.rename_file = ::rename_file;
  api.remove_file = ::remove_file;
  api.get_save_path = ::get_save_path;
  api.is_storage_available = ::is_storage_available;

  // profiler
  // api.enable_us_timer = ::enable_us_timer;
  api.get_us_timer = ::get_us_timer;
  api.get_max_us_timer = ::get_max_us_timer;

  // jpeg
  // api.decode_jpeg_buffer = ::decode_jpeg_buffer;
  // api.decode_jpeg_file = ::decode_jpeg_file;

  // launcher
  // api.launch = ::launch;
  // api.erase_game = ::erase_game;
  // api.get_type_handler_metadata = ::get_type_handler_metadata;

  api.get_launch_path = ::get_launch_path;

  // multiplayer
  api.is_multiplayer_connected = ::is_multiplayer_connected;
  api.set_multiplayer_enabled = ::set_multiplayer_enabled;
  api.send_message = ::send_multiplayer_message;

  // api.flash_to_tmp = ::flash_to_tmp;
  // api.tmp_file_closed = ::tmp_file_closed;

  api.get_metadata = ::get_metadata;

  init_led();
  init_display();
  init_input();
  init_fs();
  init_usb();

#if defined(DISPLAY_SCANVIDEO)
  multicore_launch_core1(core1_main);
#endif

  blit::set_screen_mode(ScreenMode::lores);

  blit::render = ::render;
  blit::update = ::update;

  init_audio();

  // user init
  ::init();

  while(true) {
    auto now = ::now();
    update_display(now);
    update_input();
    int ms_to_next_update = tick(::now());
    update_audio(now);
    update_led();
    update_usb();

    if(ms_to_next_update > 1 && !display_render_needed())
      best_effort_wfe_or_timeout(make_timeout_time_ms(ms_to_next_update - 1));
  }

  return 0;
}

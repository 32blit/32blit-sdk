#include <stdio.h>

#include "pico/stdlib.h"

#include "audio.hpp"
#include "input.hpp"
#include "st7789.hpp"

#include "engine/api_private.hpp"
#include "engine/engine.hpp"
#include "graphics/surface.hpp"

using namespace blit;

uint8_t screen_fb[240 * 240 * 2];
static Surface lores_screen(screen_fb, PixelFormat::RGB565, Size(160, 120));
static Surface hires_screen(screen_fb, PixelFormat::RGB565, Size(240, 240));
//static Surface hires_palette_screen(screen_fb, PixelFormat::P, Size(320, 240));

static blit::AudioChannel channels[CHANNEL_COUNT];

#ifdef DISPLAY_ST7789
pimoroni::ST7789 st7789(240, 240, (uint16_t *)screen_fb);
#endif

ScreenMode cur_screen_mode = ScreenMode::lores;

static Surface &set_screen_mode(ScreenMode mode) {
  switch(mode) {
    case ScreenMode::lores:
      screen = lores_screen;
      // window
#ifdef DISPLAY_ST7789
      st7789.set_window(40, 60, 160, 120);
#endif
      break;

    case ScreenMode::hires:
      screen = hires_screen;
#ifdef DISPLAY_ST7789
      st7789.set_window(0, 0, 240, 240);
#endif
      break;

    //case ScreenMode::hires_palette:
    //  screen = hires_palette_screen;
    //  break;
  }

  cur_screen_mode = mode;

  return blit::screen;
}

static uint32_t now() {
  return to_ms_since_boot(get_absolute_time());
}

static void *open_file(const std::string &, int) {
  return nullptr; // stub
}

static int32_t read_file(void *, uint32_t, uint32_t, char *) {
  return -1; // stub
}

static int32_t write_file(void *, uint32_t, uint32_t, const char *) {
  return -1; // stub
}

static const char *get_save_path() {
  return ""; // stub
}

static bool is_storage_available() {
  return false;
}

static uint32_t get_us_timer() {
  return to_us_since_boot(get_absolute_time());
}

static uint32_t get_max_us_timer() {
  return 0xFFFFFFFF; // it's a 64bit timer...
}

// user funcs
void init();
void render(uint32_t);
void update(uint32_t);

int main() {
  stdio_init_all();

  api.channels = ::channels;

  api.set_screen_mode = ::set_screen_mode;
  // api.set_screen_palette = ::set_screen_palette;
  api.now = ::now;
  // api.random = ::random;
  // api.exit = ::exit;

  // serial debug
  // api.debug = ::debug;

  // files
  api.open_file = ::open_file;
  api.read_file = ::read_file;
  api.write_file = ::write_file;
  // api.close_file = ::close_file;
  // api.get_file_length = ::get_file_length;
  // api.list_files = ::list_files;
  // api.file_exists = ::file_exists;
  // api.directory_exists = ::directory_exists;
  // api.create_directory = ::create_directory;
  // api.rename_file = ::rename_file;
  // api.remove_file = ::remove_file;
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

  // api.get_launch_path = ::get_launch_path;

  // multiplayer
  // api.is_multiplayer_connected = ::is_multiplayer_connected;
  // api.set_multiplayer_enabled = ::set_multiplayer_enabled;
  // api.send_message = ::send_message;
  // api.message_received = ::message_received;

  // api.flash_to_tmp = ::flash_to_tmp;
  // api.tmp_file_closed = ::tmp_file_closed;

#ifdef DISPLAY_ST7789
  st7789.init();
  st7789.clear();
#endif

  init_input();

  ::set_screen_mode(ScreenMode::lores);

  blit::render = ::render;
  blit::update = ::update;

  init_audio();

  // user init
  ::init();

  uint32_t last_render = 0;

  while(true) {
    update_input();
    tick(::now());
    update_audio();

    auto now = ::now();

#ifdef DISPLAY_ST7789
    if(now - last_render >= 20 && !st7789.dma_is_busy()) {
      ::render(now);
      st7789.update();
      last_render = now;
    }
#endif
  }

  return 0;
}

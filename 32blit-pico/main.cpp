#include <stdio.h>
#include <random>

#include "hardware/clocks.h"
#include "hardware/structs/rosc.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

#ifdef DISPLAY_SCANVIDEO
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#endif

#include "audio.hpp"
#include "config.h"
#include "file.hpp"
#include "input.hpp"
#include "led.hpp"
#include "st7789.hpp"
#include "usb.hpp"

#include "engine/api_private.hpp"
#include "engine/engine.hpp"
#include "graphics/surface.hpp"

using namespace blit;

#ifdef DISPLAY_ST7789
static const int lores_page_size = (ST7789_WIDTH / 2) * (ST7789_HEIGHT / 2) * 2;

#if ALLOW_HIRES
uint8_t screen_fb[ST7789_WIDTH * ST7789_HEIGHT * 2];
#else
uint8_t screen_fb[lores_page_size * 2]; // double-buffered
#endif

static Surface lores_screen(screen_fb, PixelFormat::RGB565, Size(ST7789_WIDTH / 2, ST7789_HEIGHT / 2));
static Surface hires_screen(screen_fb, PixelFormat::RGB565, Size(ST7789_WIDTH, ST7789_HEIGHT));
//static Surface hires_palette_screen(screen_fb, PixelFormat::P, Size(320, 240));
#elif defined(DISPLAY_SCANVIDEO)
uint8_t screen_fb[160 * 120 * 4];
static Surface lores_screen(screen_fb, PixelFormat::RGB565, Size(160, 120));
#endif

static blit::AudioChannel channels[CHANNEL_COUNT];

#ifdef DISPLAY_ST7789
#ifdef PIMORONI_PICOSYSTEM
// non-default pins
// TODO: clean this up?
pimoroni::ST7789 st7789(240, 240, (uint16_t *)screen_fb, 5, 9, 6, 7, 12, 8, 4);
#else
pimoroni::ST7789 st7789(ST7789_WIDTH, ST7789_HEIGHT, (uint16_t *)screen_fb);
#endif
static bool have_vsync = false;
#endif

ScreenMode cur_screen_mode = ScreenMode::lores;
// double buffering for lores
static volatile int buf_index = 0;

static volatile bool do_render = true;

static Surface &set_screen_mode(ScreenMode mode) {
  switch(mode) {
    case ScreenMode::lores:
      screen = lores_screen;
      // window
#ifdef DISPLAY_ST7789
      if(have_vsync)
        do_render = true; // prevent starting an update during switch

      st7789.set_pixel_double(true);
#endif
      break;

    case ScreenMode::hires:
#if defined(DISPLAY_ST7789) && ALLOW_HIRES
      if(have_vsync)
        do_render = true;

      screen = hires_screen;
      st7789.frame_buffer = (uint16_t *)screen_fb;
      st7789.set_pixel_double(false);
#else
      return screen;
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

#ifdef DISPLAY_ST7789
void vsync_callback(uint gpio, uint32_t events) {
  if(!do_render && !st7789.dma_is_busy()) {
    st7789.update();
    do_render = true;
  }
}
#endif

#ifdef DISPLAY_SCANVIDEO

static void fill_scanline_buffer(struct scanvideo_scanline_buffer *buffer) {
  static uint32_t postamble[] = {
    0x0000u | (COMPOSABLE_EOL_ALIGN << 16)
  };

  int w = screen.bounds.w;

  buffer->data[0] = 4;
  buffer->data[1] = host_safe_hw_ptr(buffer->data + 8);
  buffer->data[2] = (w - 4) / 2; // first four pixels are handled separately
  uint16_t *pixels = ((uint16_t *)screen_fb) + buf_index * (160 * 120) + scanvideo_scanline_number(buffer->scanline_id) * w;
  buffer->data[3] = host_safe_hw_ptr(pixels + 4);
  buffer->data[4] = count_of(postamble);
  buffer->data[5] = host_safe_hw_ptr(postamble);
  buffer->data[6] = 0;
  buffer->data[7] = 0;
  buffer->data_used = 8;

  // 3 pixel run followed by main run, consuming the first 4 pixels
  buffer->data[8] = (pixels[0] << 16u) | COMPOSABLE_RAW_RUN;
  buffer->data[9] = (pixels[1] << 16u) | 0;
  buffer->data[10] = (COMPOSABLE_RAW_RUN << 16u) | pixels[2];
  buffer->data[11] = (((w - 3) + 1 - 3) << 16u) | pixels[3]; // note we add one for the black pixel at the end
}

static int64_t timer_callback(alarm_id_t alarm_id, void *user_data) {
  static int last_frame = 0;
  struct scanvideo_scanline_buffer *buffer = scanvideo_begin_scanline_generation(false);
  while (buffer) {
    fill_scanline_buffer(buffer);
    scanvideo_end_scanline_generation(buffer);

    auto next_frame = scanvideo_frame_number(scanvideo_get_next_scanline_id());
    if(next_frame != last_frame) {
    //if(scanvideo_in_vblank() && !do_render) {
      do_render = true;
      last_frame = next_frame;
      break;
    }

    buffer = scanvideo_begin_scanline_generation(false);
  }


  return 100;
}
#endif

int main() {
#if OVERCLOCK_250
  set_sys_clock_khz(250000, false);
#endif

  stdio_init_all();

  api.channels = ::channels;

  api.set_screen_mode = ::set_screen_mode;
  // api.set_screen_palette = ::set_screen_palette;
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

#ifdef DISPLAY_ST7789
  bool backlight_enabled = false;
  st7789.init();
  st7789.clear();

  have_vsync = st7789.vsync_callback(vsync_callback);
#endif

#ifdef DISPLAY_SCANVIDEO
  //scanvideo_setup(&vga_mode_320x240_60); // not quite
  scanvideo_setup(&vga_mode_160x120_60);
  scanvideo_timing_enable(true);
  add_alarm_in_us(100, timer_callback, nullptr, true);
#endif

  init_input();
  init_fs();
  init_usb();

  ::set_screen_mode(ScreenMode::lores);

  blit::render = ::render;
  blit::update = ::update;

  init_audio();

  // user init
  ::init();

  uint32_t last_render = 0;

  while(true) {
    auto now = ::now();

#ifdef DISPLAY_ST7789
    if((do_render || (!have_vsync && now - last_render >= 20)) && (cur_screen_mode == ScreenMode::lores || !st7789.dma_is_busy())) {
      if(cur_screen_mode == ScreenMode::lores) {
        buf_index ^= 1;

        screen.data = screen_fb + (buf_index) * lores_page_size;
        st7789.frame_buffer = (uint16_t *)screen.data;
      }

      ::render(now);

      if(!have_vsync)
        st7789.update();

      if(last_render && !backlight_enabled) {
        // the first render should have made it to the screen at this point
        st7789.set_backlight(255);
        backlight_enabled = true;
      }

      last_render = now;
      do_render = false;
    }

#elif defined(DISPLAY_SCANVIDEO)
    if(do_render) {
      screen.data = screen_fb + (buf_index ^ 1) * (160 * 120 * 2); // only works because there's no "firmware" here
      ::render(now);
      buf_index ^= 1;
      do_render = false;
    }
#endif
    update_input();
    int ms_to_next_update = tick(::now());
    update_audio(now);
    update_led();
    update_usb();

    if(ms_to_next_update > 1)
      best_effort_wfe_or_timeout(make_timeout_time_ms(ms_to_next_update - 1));
  }

  return 0;
}

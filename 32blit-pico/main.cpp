#include <stdio.h>

#include "hardware/clocks.h"
#include "pico/stdlib.h"

#ifdef DISPLAY_SCANVIDEO
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#endif

#include "audio.hpp"
#include "input.hpp"
#include "led.hpp"
#include "st7789.hpp"

#include "engine/api_private.hpp"
#include "engine/engine.hpp"
#include "graphics/surface.hpp"

using namespace blit;

#ifdef DISPLAY_ST7789
uint8_t screen_fb[240 * 240 * 2];
static Surface lores_screen(screen_fb, PixelFormat::RGB565, Size(120, 120));
static Surface hires_screen(screen_fb, PixelFormat::RGB565, Size(240, 240));
//static Surface hires_palette_screen(screen_fb, PixelFormat::P, Size(320, 240));
#elif defined(DISPLAY_SCANVIDEO)
uint8_t screen_fb[160 * 120 * 4];
static Surface lores_screen(screen_fb, PixelFormat::RGB565, Size(160, 120));
#endif

static blit::AudioChannel channels[CHANNEL_COUNT];

#ifdef DISPLAY_ST7789
#ifdef PIMORONI_PICOSYSTEM
pimoroni::ST7789 st7789(240, 240, (uint16_t *)screen_fb, 5, 9, 6, 7, 12, 8, 4);
#else
pimoroni::ST7789 st7789(240, 240, (uint16_t *)screen_fb);
#endif
#endif

ScreenMode cur_screen_mode = ScreenMode::lores;

static Surface &set_screen_mode(ScreenMode mode) {
  switch(mode) {
    case ScreenMode::lores:
      screen = lores_screen;
      // window
#ifdef DISPLAY_ST7789
      st7789.set_pixel_double(true);
#endif
      break;

    case ScreenMode::hires:
#ifdef DISPLAY_ST7789
      screen = hires_screen;
      st7789.set_pixel_double(false);
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

#ifdef DISPLAY_SCANVIDEO

static volatile bool do_render = true;
static volatile int buf_index = 0;

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
  set_sys_clock_khz(250000, false);

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

  init_led();

#ifdef DISPLAY_ST7789
  st7789.init();
  st7789.clear();
#endif

#ifdef DISPLAY_SCANVIDEO
  //scanvideo_setup(&vga_mode_320x240_60); // not quite
  scanvideo_setup(&vga_mode_160x120_60);
  scanvideo_timing_enable(true);
  add_alarm_in_us(100, timer_callback, nullptr, true);
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
    update_led();

    auto now = ::now();

#ifdef DISPLAY_ST7789
    if(now - last_render >= 20 && !st7789.dma_is_busy()) {
      ::render(now);
      st7789.update();
      last_render = now;
    }
#elif defined(DISPLAY_SCANVIDEO)
    if(do_render) {
      screen.data = screen_fb + (buf_index ^ 1) * (160 * 120 * 2); // only works because there's no "firmware" here
      ::render(now);
      buf_index ^= 1;
      do_render = false;
    }
#endif
  }

  return 0;
}

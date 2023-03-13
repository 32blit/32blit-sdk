#include "display.hpp"

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "pico/time.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"

using namespace blit;

// double buffering for lores
static uint8_t *cur_display_buffer = nullptr;

static volatile bool do_render = true;

static bool do_render_soon = false; // slightly delayed to handle the queue

static void fill_scanline_buffer(struct scanvideo_scanline_buffer *buffer) {
  static uint32_t postamble[] = {
    0x0000u | (COMPOSABLE_EOL_ALIGN << 16)
  };

  int w = cur_surf_info.bounds.w;

  buffer->data[0] = 4;
  buffer->data[1] = host_safe_hw_ptr(buffer->data + 8);
  buffer->data[2] = (w - 4) / 2; // first four pixels are handled separately
  uint16_t *pixels = (uint16_t *)cur_display_buffer + scanvideo_scanline_number(buffer->scanline_id) * w;
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

void init_display() {
  // channel 0 get claimed later, channel 3 doesn't get claimed, but does get used
  // reserve them so out claims don't conflict
  dma_claim_mask(1 << 0 | 1 << 3);

  // PIO SMs that get claimed later
  pio_claim_sm_mask(pio0, 1 << 0 | 1 << 3);
}

void update_display(uint32_t time) {
  if(do_render) {
    ::render(time);
    do_render = false;
  }
}

void init_display_core1() {
  dma_unclaim_mask(1 << 0 | 1 << 3);
  pio_sm_unclaim(pio0,  0);
  pio_sm_unclaim(pio0,  3);

  // no mode switching yet
#if ALLOW_HIRES
#if DISPLAY_HEIGHT == 160 // extra middle mode
  scanvideo_setup(&vga_mode_213x160_60);
#else
  scanvideo_setup(&vga_mode_320x240_60);
#endif
#else
  scanvideo_setup(&vga_mode_160x120_60);
#endif

  scanvideo_timing_enable(true);
}

void update_display_core1() {
  if(!cur_display_buffer)
    return;

  struct scanvideo_scanline_buffer *buffer = scanvideo_begin_scanline_generation(true);

  while (buffer) {
    fill_scanline_buffer(buffer);
    scanvideo_end_scanline_generation(buffer);

    const int height = cur_surf_info.bounds.h;

    if(scanvideo_scanline_number(buffer->scanline_id) == height - 1 && !do_render) {
      // swap buffers at the end of the frame, but don't start a render yet
      // (the last few lines of the old buffer are still in the queue)
      if(fb_double_buffer) {
        do_render_soon = true;
        std::swap(screen.data, cur_display_buffer);
      } else {
        // hires is single buffered and disabled by default
        // rendering correctly is the user's problem
        do_render = true;
      }
      break;
    } else if(do_render_soon && scanvideo_scanline_number(buffer->scanline_id) == PICO_SCANVIDEO_SCANLINE_BUFFER_COUNT - 1) {
      // should be safe to reuse old buffer now
      do_render = do_render_soon;
      do_render_soon = false;
    }

    buffer = scanvideo_begin_scanline_generation(false);
  }
}

bool display_render_needed() {
  return do_render;
}

bool display_mode_supported(blit::ScreenMode new_mode, const blit::SurfaceTemplate &new_surf_template) {
  if(new_surf_template.format != blit::PixelFormat::RGB565)
    return false;

  blit::Size expected_bounds(DISPLAY_WIDTH, DISPLAY_HEIGHT);

  if(new_surf_template.bounds == expected_bounds || new_surf_template.bounds == expected_bounds / 2)
    return true;

  return false;
}

void display_mode_changed(blit::ScreenMode new_mode, blit::SurfaceTemplate &new_surf_template) {
  auto display_buf_base = (uint8_t *)screen_fb;

  bool use_second_buf = fb_double_buffer && screen.data == display_buf_base;
  cur_display_buffer = use_second_buf ? display_buf_base + get_display_page_size() : display_buf_base;
}

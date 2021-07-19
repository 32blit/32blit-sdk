#pragma once

#include "hardware/pio.h"
#include "hardware/gpio.h"

namespace pimoroni {

  class ST7789 {
    PIO pio = pio0;
    uint pio_sm = 0;
    uint pio_offset = 0, pio_double_offset = 0;

    uint32_t dma_channel;

    // screen properties
    uint16_t width;
    uint16_t height;
    uint16_t row_stride;

    uint16_t win_w, win_h; // window size

    // interface pins with our standard defaults where appropriate
    int8_t cs     = 17;
    int8_t dc     = 16;
    int8_t sck    = 18;
    int8_t mosi   = 19;
    int8_t bl     = 20;
    int8_t vsync  = -1; // only available on some products
    int8_t reset  = -1;

    bool write_mode = false; // in RAMWR
    bool pixel_double = false;
    uint16_t *upd_frame_buffer = nullptr;

  public:
    // frame buffer where pixel data is stored
    uint16_t *frame_buffer;

  public:
    ST7789(uint16_t width, uint16_t height, uint16_t *frame_buffer) :
      width(width), height(height), win_w(width), win_h(height), frame_buffer(frame_buffer) {}

    ST7789(uint16_t width, uint16_t height, uint16_t *frame_buffer,
           uint8_t cs, uint8_t dc, uint8_t sck, uint8_t mosi, uint8_t bl, uint8_t vsync, uint8_t reset) :
      width(width), height(height), win_w(width), win_h(height),
      cs(cs), dc(dc), sck(sck), mosi(mosi), bl(bl), vsync(vsync), reset(reset), frame_buffer(frame_buffer) {}

    void init(bool auto_init_sequence = true);

    void command(uint8_t command, size_t len = 0, const char *data = NULL);
    bool vsync_callback(gpio_irq_callback_t callback);
    void update(bool dont_block = false);
    void set_backlight(uint8_t brightness);

    void set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void set_pixel_double(bool pd);

    void clear();

    bool dma_is_busy();

  private:
    void prepare_write();

    friend void st7789_dma_irq_handler();
  };

}

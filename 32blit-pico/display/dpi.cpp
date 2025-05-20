#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "pico/time.h"

#include "display.hpp"
#include "display_commands.hpp"

#include "config.h"

#include "dpi.pio.h"

enum ST7701Reg {
  // Command2_BK0
  PVGAMCTRL = 0xB0,  // Positive Voltage Gamma Control
  NVGAMCTRL = 0xB1,  // Negative Voltage Gamma Control
  DGMEN = 0xB8,   // Digital Gamma Enable
  DGMLUTR = 0xB9, // Digital Gamma LUT for Red
  DGMLUTB = 0xBA, // Digital Gamma Lut for Blue
  LNESET = 0xC0,  // Display Line Setting
  PORCTRL = 0xC1, // Porch Control
  INVSET = 0xC2,  // Inversion Selection & Frame Rate Control
  RGBCTRL = 0xC3, // RGB Control
  PARCTRL = 0xC5, // Partial Mode Control
  SDIR = 0xC7,    // X-direction Control
  PDOSET = 0xC8,  // Pseudo-Dot Inversion Diving Setting
  COLCTRL = 0xCD, // Colour Control
  SRECTRL = 0xE0, // Sunlight Readable Enhancement
  NRCTRL = 0xE1,  // Noise Reduce Control
  SECTRL = 0xE2,  // Sharpness Control
  CCCTRL = 0xE3,  // Color Calibration Control
  SKCTRL = 0xE4,  // Skin Tone Preservation Control
  // Command2_BK1
  VHRS = 0xB0,    // Vop amplitude
  VCOMS = 0xB1,   // VCOM amplitude
  VGHSS = 0xB2,   // VGH voltage
  TESTCMD = 0xB3, // TEST command
  VGLS = 0xB5,    // VGL voltage
  VRHDV = 0xB6,   // VRH_DV voltage
  PWCTRL1 = 0xB7, // Power Control 1
  PWCTRL2 = 0xB8, // Power Control 2
  PCLKS1 = 0xBA,  // Power pumping clock selection 1
  PCLKS2 = 0xBC,  // Power pumping clock selection 2
  PDR1 = 0xC1,    // Source pre_drive timing set 1
  PDR2 = 0xC2,    // Source pre_drive timing set 2
  // Command2_BK3
  NVMEN = 0xC8,    // NVM enable
  NVMSET = 0xCA,   // NVM manual control
  PROMACT = 0xCC,  // NVM program active
  // Other
  CND2BKxSEL = 0xFF,
};

#ifndef DPI_DATA_PIN_BASE
#define DPI_DATA_PIN_BASE 0
#endif

#ifndef DPI_SYNC_PIN_BASE
#define DPI_SYNC_PIN_BASE 16
#endif

// mode (default to 640x480)
#ifndef DPI_MODE_CLOCK
#define DPI_MODE_CLOCK 25000000
#endif

#ifndef DPI_MODE_H_SYNC_POLARITY
#define DPI_MODE_H_SYNC_POLARITY 0
#endif
#ifndef DPI_MODE_H_FRONT_PORCH
#define DPI_MODE_H_FRONT_PORCH   16
#endif
#ifndef DPI_MODE_H_SYNC_WIDTH
#define DPI_MODE_H_SYNC_WIDTH    96
#endif
#ifndef DPI_MODE_H_BACK_PORCH
#define DPI_MODE_H_BACK_PORCH    48
#endif
#ifndef DPI_MODE_H_ACTIVE_PIXELS
#define DPI_MODE_H_ACTIVE_PIXELS 640
#endif

#ifndef DPI_MODE_V_SYNC_POLARITY
#define DPI_MODE_V_SYNC_POLARITY 0
#endif
#ifndef DPI_MODE_V_FRONT_PORCH
#define DPI_MODE_V_FRONT_PORCH   10
#endif
#ifndef DPI_MODE_V_SYNC_WIDTH
#define DPI_MODE_V_SYNC_WIDTH    2
#endif
#ifndef DPI_MODE_V_BACK_PORCH
#define DPI_MODE_V_BACK_PORCH    33
#endif
#ifndef DPI_MODE_V_ACTIVE_LINES
#define DPI_MODE_V_ACTIVE_LINES  480
#endif

static_assert(DPI_MODE_H_ACTIVE_PIXELS % DISPLAY_WIDTH == 0);
static_assert(DPI_MODE_V_ACTIVE_LINES % DISPLAY_HEIGHT == 0);

#define MODE_V_TOTAL_LINES  ( \
  DPI_MODE_V_FRONT_PORCH + DPI_MODE_V_SYNC_WIDTH + \
  DPI_MODE_V_BACK_PORCH  + DPI_MODE_V_ACTIVE_LINES \
)

// DMA logic

#define DPI_DMA_CH_BASE 0
#define DPI_NUM_DMA_CHANNELS 2

static uint8_t cur_dma_ch = DPI_DMA_CH_BASE;

static PIO pio = pio0;
static uint8_t timing_sm, data_sm;
static uint8_t data_program_offset;

// pixel/line repeat
static uint16_t line_width = 0;
static uint8_t v_repeat = 0;
static uint8_t new_v_repeat = 0;

static uint data_scanline = DPI_NUM_DMA_CHANNELS;
static uint timing_scanline = 0;
static uint8_t timing_offset = 0;

static bool started = false;
static volatile bool do_render = true;
static volatile bool need_mode_change = false;
static uint8_t reconfigure_data_pio = 0;
static uint8_t *cur_display_buffer = nullptr;

static uint32_t active_line_timings[4];
static uint32_t vblank_line_timings[4];
static uint32_t vsync_line_timings[4];

// assumes data SM is idle
static inline void update_h_repeat() {
  // update Y register
  pio_sm_put(pio, data_sm, line_width - 1);
  pio_sm_exec(pio, data_sm, pio_encode_out(pio_y, 32));

  // patch loop delay for repeat
  int h_repeat = DPI_MODE_H_ACTIVE_PIXELS / line_width;
  auto delay = (h_repeat - 1) * 2;

#ifdef DPI_BIT_REVERSE
  auto offset = dpi_data_reversed_16_offset_data_loop_delay;
  auto instr = dpi_data_reversed_16_program.instructions[offset];
  delay *= 2;
#else
  auto offset = dpi_data_16_offset_data_loop_delay;
  auto instr = dpi_data_16_program.instructions[offset];
#endif

  // need to add the program offset as it's a jump
  pio->instr_mem[data_program_offset + offset] = (instr | pio_encode_delay(delay)) + data_program_offset;
}

static void __not_in_flash_func(dma_irq_handler)() {
  // this only covers active lines

  dma_channel_hw_t *ch = &dma_hw->ch[cur_dma_ch];
  dma_hw->intr = 1u << cur_dma_ch;

  if(cur_dma_ch + 1 == DPI_DMA_CH_BASE + DPI_NUM_DMA_CHANNELS)
    cur_dma_ch = DPI_DMA_CH_BASE;
  else
    cur_dma_ch++;

  if(data_scanline == DPI_MODE_V_ACTIVE_LINES) {
    // new frame, swap buffers
    data_scanline = 0;

    if(!do_render) {
      if(fb_double_buffer)
        std::swap(blit::screen.data, cur_display_buffer);
      do_render = true;
    }

    // set h/v shift
    if(need_mode_change) {
      if(line_width != cur_surf_info.bounds.w) {
        reconfigure_data_pio = (ch - dma_hw->ch) + 1;
        hw_clear_bits(&ch->al1_ctrl, DMA_CH0_CTRL_TRIG_EN_BITS); // clear enable so line 0 won't start
      }

      v_repeat = new_v_repeat;
      line_width = cur_surf_info.bounds.w;

      need_mode_change = false;
    }
  } else if(reconfigure_data_pio) {
    // this should be the point where the last line finished (in vblank) and we would start line 0, but we disabled it
    // reconfigure the PIO before re-enabling it
    int prev_chan = reconfigure_data_pio - 1;

    while(pio->sm[data_sm].addr != data_program_offset); // wait until we've returned to waiting for irq

    update_h_repeat();

    // resume
    hw_set_bits(&dma_hw->ch[prev_chan].ctrl_trig, DMA_CH0_CTRL_TRIG_EN_BITS);
    reconfigure_data_pio = 0;
  }

  // setup next line DMA
  int display_line = data_scanline / v_repeat;
  auto w = line_width;
  auto fb_line_ptr = reinterpret_cast<uint16_t *>(cur_display_buffer) + display_line * w;

  ch->read_addr = uintptr_t(fb_line_ptr);
  ch->transfer_count = w / 2;

  data_scanline++;
}

static void __not_in_flash_func(pio_timing_irq_handler)() {
  while(!(pio->fstat & (1 << (PIO_FSTAT_TXFULL_LSB + timing_sm)))) {
    if(timing_scanline >= DPI_MODE_V_FRONT_PORCH && timing_scanline < DPI_MODE_V_FRONT_PORCH + DPI_MODE_V_SYNC_WIDTH)
      pio_sm_put(pio, timing_sm, vsync_line_timings[timing_offset]); // v sync
    else if(timing_scanline < DPI_MODE_V_FRONT_PORCH + DPI_MODE_V_SYNC_WIDTH + DPI_MODE_V_BACK_PORCH)
      pio_sm_put(pio, timing_sm, vblank_line_timings[timing_offset]); // v blank
    else
      pio_sm_put(pio, timing_sm, active_line_timings[timing_offset]); // active

    if(++timing_offset == std::size(active_line_timings)) {
      timing_offset = 0;

      if(++timing_scanline == MODE_V_TOTAL_LINES)
        timing_scanline = 0;
    }
  }
}

#ifdef DPI_SPI_INIT
static void command(uint8_t reg, size_t len = 0, const char *data = nullptr) {
  gpio_put(LCD_CS_PIN, 0);

#if LCD_DC_PIN != -1
  gpio_put(LCD_DC_PIN, 0); // command
  spi_write_blocking(DPI_SPI_INIT, &reg, 1);

  if(data) {
    gpio_put(LCD_DC_PIN, 1); // data
    spi_write_blocking(DPI_SPI_INIT, (const uint8_t *)data, len);
  }
#else
  uint16_t v = reg;
  spi_write16_blocking(DPI_SPI_INIT, &v, 1);

  if(data) {
    for(size_t i = 0; i < len; i++) {
      v = data[i] | 0x100;
      spi_write16_blocking(DPI_SPI_INIT, &v, 1);
    }
  }
#endif

  gpio_put(LCD_CS_PIN, 1);
}
#endif

static void init_display_spi() {
#ifdef DPI_SPI_INIT
  spi_init(DPI_SPI_INIT, 1 * 1000 * 1000);
  gpio_set_function(LCD_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);

  // init CS
  gpio_init(LCD_CS_PIN);
  gpio_set_dir(LCD_CS_PIN, GPIO_OUT);
  gpio_put(LCD_CS_PIN, 1);

  bi_decl_if_func_used(bi_1pin_with_name(LCD_MOSI_PIN, "Display TX"));
  bi_decl_if_func_used(bi_1pin_with_name(LCD_SCK_PIN, "Display SCK"));
  bi_decl_if_func_used(bi_1pin_with_name(LCD_CS_PIN, "Display CS"));

#if LCD_DC_PIN != -1
  // init D/C
  gpio_init(LCD_DC_PIN);
  gpio_set_dir(LCD_DC_PIN, GPIO_OUT);

  bi_decl_if_func_used(bi_1pin_with_name(LCD_DC_PIN, "Display D/C"));
#else
  // configure for 9 bit if no D/C pin
  spi_set_format(DPI_SPI_INIT, 9, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
#endif

#ifdef LCD_RESET_PIN
  gpio_init(LCD_RESET_PIN);
  gpio_set_dir(LCD_RESET_PIN, GPIO_OUT);

  sleep_ms(15);
  gpio_put(LCD_RESET_PIN, 1);
  sleep_ms(15);

  bi_decl_if_func_used(bi_1pin_with_name(LCD_RESET_PIN, "Display Reset"));
#endif

#ifdef DPI_ST7701
  command(MIPIDCS::SoftReset);

  sleep_ms(150);

  // Commmand 2 BK0 - kinda a page select
  command(ST7701Reg::CND2BKxSEL, 5, "\x77\x01\x00\x00\x10");

  command(ST7701Reg::LNESET, 2, "\x3b\x00");   // (59 + 1) * 8 = 480 lines
  command(ST7701Reg::PORCTRL, 2, "\x0d\x02");  // Display porch settings: 13 VBP, 2 VFP (these should not be changed)
  command(ST7701Reg::INVSET, 2, "\x31\x01");
  command(ST7701Reg::COLCTRL, 1, "\x08");      // LED polarity reversed
  command(ST7701Reg::PVGAMCTRL, 16, "\x00\x11\x18\x0e\x11\x06\x07\x08\x07\x22\x04\x12\x0f\xaa\x31\x18");
  command(ST7701Reg::NVGAMCTRL, 16, "\x00\x11\x19\x0e\x12\x07\x08\x08\x08\x22\x04\x11\x11\xa9\x32\x18");
  command(ST7701Reg::RGBCTRL, 3, "\x80\x2e\x0e");  // HV mode, H and V back porch + sync


  // Command 2 BK1 - Voltages and power and stuff
  command(ST7701Reg::CND2BKxSEL, 5, "\x77\x01\x00\x00\x11");
  command(ST7701Reg::VHRS, 1, "\x60");    // 4.7375v
  command(ST7701Reg::VCOMS, 1, "\x32");   // 0.725v
  command(ST7701Reg::VGHSS, 1, "\x07");   // 15v
  command(ST7701Reg::TESTCMD, 1, "\x80"); // y tho?
  command(ST7701Reg::VGLS, 1, "\x49");    // -10.17v
  command(ST7701Reg::PWCTRL1, 1, "\x85"); // Middle/Min/Min bias
  command(ST7701Reg::PWCTRL2, 1, "\x21"); // 6.6 / -4.6
  command(ST7701Reg::PDR1, 1, "\x78");    // 1.6uS
  command(ST7701Reg::PDR2, 1, "\x78");    // 6.4uS

  // Begin Forbidden Knowledge
  // This sequence is probably specific to TL040WVS03CT15-H1263A.
  // It is not documented in the ST7701s datasheet.
  // TODO: 👇 W H A T ! ? 👇
  command(0xE0, 3, "\x00\x1b\x02");
  command(0xE1, 11, "\x08\xa0\x00\x00\x07\xa0\x00\x00\x00\x44\x44");
  command(0xE2, 12, "\x11\x11\x44\x44\xed\xa0\x00\x00\xec\xa0\x00\x00");
  command(0xE3, 4, "\x00\x00\x11\x11");
  command(0xE4, 2, "\x44\x44");
  command(0xE5, 16, "\x0a\xe9\xd8\xa0\x0c\xeb\xd8\xa0\x0e\xed\xd8\xa0\x10\xef\xd8\xa0");
  command(0xE6, 4, "\x00\x00\x11\x11");
  command(0xE7, 2, "\x44\x44");
  command(0xE8, 16, "\x09\xe8\xd8\xa0\x0b\xea\xd8\xa0\x0d\xec\xd8\xa0\x0f\xee\xd8\xa0");
  command(0xEB, 7, "\x02\x00\xe4\xe4\x88\x00\x40");
  command(0xEC, 2, "\x3c\x00");
  command(0xED, 16, "\xab\x89\x76\x54\x02\xff\xff\xff\xff\xff\xff\x20\x45\x67\x98\xba");
  command(0x36, 1, "\x00");

  // Command 2 BK3
  command(ST7701Reg::CND2BKxSEL, 5, "\x77\x01\x00\x00\x13");
  command(0xE5, 1, "\xe4");
  // End Forbidden Knowledge

  command(ST7701Reg::CND2BKxSEL, 5, "\x77\x01\x00\x00\x00");

  command(MIPIDCS::SetPixelFormat, 1, "\x66"); // (18bpp)

  uint8_t madctl = MADCTL::RGB;
  command(MIPIDCS::SetAddressMode, 1, (char *)&madctl);

  command(MIPIDCS::EnterInvertMode);
  sleep_ms(1);
  command(MIPIDCS::ExitSleepMode);
  sleep_ms(120);
  command(MIPIDCS::DisplayOn);
#endif
#endif
}

void init_display() {
  // send init commands if needed
  init_display_spi();

  // setup timing buffers
  auto encode_timing = [](uint16_t instr, bool vsync, bool hsync, bool de, int delay) {
    // instr needs sideset 0, but that's just a zero
    return instr                                   << 16
         | (delay - 3)                             <<  3 // two cycles from setup, one for the first loop iteration
         //| (de ? 1 : 0) << 2 // TODO
         | (vsync == DPI_MODE_V_SYNC_POLARITY ? 1 : 0) <<  1
         | (hsync == DPI_MODE_H_SYNC_POLARITY ? 1 : 0) <<  0;
  };

  //                                     instr                           vbl    hbl    de     delay
  active_line_timings[0] = encode_timing(pio_encode_nop(),               false, true,  false, DPI_MODE_H_SYNC_WIDTH);
  active_line_timings[1] = encode_timing(pio_encode_nop(),               false, false, false, DPI_MODE_H_BACK_PORCH);
  active_line_timings[2] = encode_timing(pio_encode_irq_set(false, 4),   false, false, true,  DPI_MODE_H_ACTIVE_PIXELS);
  active_line_timings[3] = encode_timing(pio_encode_irq_clear(false, 4), false, false, false, DPI_MODE_H_FRONT_PORCH);

  vblank_line_timings[0] = encode_timing(pio_encode_nop(),               false, true,  false, DPI_MODE_H_SYNC_WIDTH);
  vblank_line_timings[1] = encode_timing(pio_encode_nop(),               false, false, false, DPI_MODE_H_BACK_PORCH);
  vblank_line_timings[2] = encode_timing(pio_encode_nop(),               false, false, false, DPI_MODE_H_ACTIVE_PIXELS);
  vblank_line_timings[3] = encode_timing(pio_encode_nop(),               false, false, false, DPI_MODE_H_FRONT_PORCH);

  vsync_line_timings[0]  = encode_timing(pio_encode_nop(),               true,  true,  false, DPI_MODE_H_SYNC_WIDTH);
  vsync_line_timings[1]  = encode_timing(pio_encode_nop(),               true,  false, false, DPI_MODE_H_BACK_PORCH);
  vsync_line_timings[2]  = encode_timing(pio_encode_nop(),               true,  false, false, DPI_MODE_H_ACTIVE_PIXELS);
  vsync_line_timings[3]  = encode_timing(pio_encode_nop(),               true,  false, false, DPI_MODE_H_FRONT_PORCH);

  // setup timing program
  int num_sync_pins = 2; // h/v sync
  const int num_data_pins = 16; // assume 16-bit/565

  int pio_offset = pio_add_program(pio, &dpi_timing_program);

  // allocate data first so unassigned clock pin doesn't cause problems
  data_sm = pio_claim_unused_sm(pio, true);
  timing_sm = pio_claim_unused_sm(pio, true);

  pio_sm_config cfg = dpi_timing_program_get_default_config(pio_offset);

  const int clkdiv = clock_get_hz(clk_sys) / (DPI_MODE_CLOCK * 2);
  assert(clock_get_hz(clk_sys) / clkdiv == DPI_MODE_CLOCK * 2);
  sm_config_set_clkdiv_int_frac(&cfg, clkdiv, 0);

  sm_config_set_out_shift(&cfg, false, true, 32);
  sm_config_set_out_pins(&cfg, DPI_SYNC_PIN_BASE, num_sync_pins);
  sm_config_set_fifo_join(&cfg, PIO_FIFO_JOIN_TX);
#ifdef DPI_CLOCK_PIN
  sm_config_set_sideset_pins(&cfg, DPI_CLOCK_PIN);
#endif

  pio_sm_init(pio, timing_sm, pio_offset, &cfg);

  // setup data program

#ifdef DPI_BIT_REVERSE
  pio_offset = pio_add_program(pio, &dpi_data_reversed_16_program);

  cfg = dpi_data_reversed_16_program_get_default_config(pio_offset);
  assert(!(clkdiv & 1));
  sm_config_set_clkdiv_int_frac(&cfg, clkdiv / 2, 0);
#else
  pio_offset = pio_add_program(pio, &dpi_data_16_program);

  cfg = dpi_data_16_program_get_default_config(pio_offset);
  sm_config_set_clkdiv_int_frac(&cfg, clkdiv, 0);
#endif
  sm_config_set_out_shift(&cfg, true, true, 32);
  sm_config_set_in_shift(&cfg, false, false, 32);
  sm_config_set_out_pins(&cfg, DPI_DATA_PIN_BASE, num_data_pins);
  sm_config_set_fifo_join(&cfg, PIO_FIFO_JOIN_TX);

  data_program_offset = pio_offset;

  pio_sm_init(pio, data_sm, pio_offset, &cfg);

  // init Y register
  pio_sm_put(pio, data_sm, DPI_MODE_H_ACTIVE_PIXELS - 1);
  pio_sm_exec(pio, data_sm, pio_encode_out(pio_y, 32));

  // init pins
  for(int i = 0; i < num_sync_pins; i++)
    pio_gpio_init(pio, DPI_SYNC_PIN_BASE + i);

  for(int i = 0; i < num_data_pins; i++)
    pio_gpio_init(pio, DPI_DATA_PIN_BASE + i);

  pio_sm_set_consecutive_pindirs(pio, timing_sm, DPI_SYNC_PIN_BASE, num_sync_pins, true);
  pio_sm_set_consecutive_pindirs(pio, data_sm, DPI_DATA_PIN_BASE, num_data_pins, true);

  bi_decl_if_func_used(bi_pin_mask_with_name(3 << DPI_SYNC_PIN_BASE, "Display Sync"));
  bi_decl_if_func_used(bi_pin_mask_with_name(0xFFFF << DPI_DATA_PIN_BASE, "Display Data"));

#ifdef DPI_CLOCK_PIN
  pio_gpio_init(pio, DPI_CLOCK_PIN);
  pio_sm_set_consecutive_pindirs(pio, timing_sm, DPI_CLOCK_PIN, 1, true);

  bi_decl_if_func_used(bi_1pin_with_name(DPI_CLOCK_PIN, "Display Clock"));
#endif

  // setup PIO IRQ
  pio_set_irq0_source_enabled(pio, pio_interrupt_source_t(pis_sm0_tx_fifo_not_full + timing_sm), true);
  irq_set_exclusive_handler(pio_get_irq_num(pio, 0), pio_timing_irq_handler);
  irq_set_enabled(pio_get_irq_num(pio, 0), true);

  // setup data DMA
  // chain channels in a loop
  for(int i = 0; i < DPI_NUM_DMA_CHANNELS; i++) {
    dma_channel_claim(DPI_DMA_CH_BASE + i);
    dma_channel_config c;
    c = dma_channel_get_default_config(DPI_DMA_CH_BASE + i);

    int next_chan = i == (DPI_NUM_DMA_CHANNELS - 1) ? 0 : i + 1;

    channel_config_set_chain_to(&c, DPI_DMA_CH_BASE + next_chan);
    channel_config_set_dreq(&c, pio_get_dreq(pio, data_sm, true));

    dma_channel_configure(
      DPI_DMA_CH_BASE + i,
      &c,
      &pio->txf[data_sm],
      cur_display_buffer,
      DPI_MODE_H_ACTIVE_PIXELS,
      false
    );
  }

  const unsigned chan_mask = (1 << DPI_NUM_DMA_CHANNELS) - 1;

  dma_hw->ints0 = (chan_mask << DPI_DMA_CH_BASE);
  dma_hw->inte0 = (chan_mask << DPI_DMA_CH_BASE);
  irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
  irq_set_enabled(DMA_IRQ_0, true);
}

void update_display(uint32_t time) {
  if(do_render) {
    blit::render(time);

    // start dma/pio after first render
    if(!started && blit::screen.data) {
      started = true;
      dma_channel_start(DPI_DMA_CH_BASE);
      pio_set_sm_mask_enabled(pio, 1 << timing_sm | 1 << data_sm, true);
    } else if(cur_surf_info.bounds.w != line_width || new_v_repeat != v_repeat) {
      need_mode_change = true;
    }
    do_render = false;
  }
}

void init_display_core1() {
}

void update_display_core1() {
}

bool display_render_needed() {
  return do_render;
}

bool display_mode_supported(blit::ScreenMode new_mode, const blit::SurfaceTemplate &new_surf_template) {
  if(new_surf_template.format != blit::PixelFormat::RGB565)
    return false;

  auto w = new_surf_template.bounds.w;
  auto h = new_surf_template.bounds.h;

  const int min_size = 96; // clamp smallest size

  // width needs to be even
  // allow a little rounding (it'll be filled with black)
  int repeat = DPI_MODE_H_ACTIVE_PIXELS / w;
  if(w < min_size || DPI_MODE_H_ACTIVE_PIXELS % w > repeat + 1 || (w & 1))
    return false;

  if(h < min_size || DPI_MODE_V_ACTIVE_LINES % h)
    return false;

  return true;
}

void display_mode_changed(blit::ScreenMode new_mode, blit::SurfaceTemplate &new_surf_template) {
  auto display_buf_base = (uint8_t *)screen_fb;

  // prevent buffer swap while we're doing this
  do_render = true;

  bool use_second_buf = fb_double_buffer && (!blit::screen.data || blit::screen.data == display_buf_base);
  cur_display_buffer = use_second_buf ? display_buf_base + get_display_page_size() : display_buf_base;

  // avoid resetting screen.data to first buffer, causing both buffers to be the same
  if(fb_double_buffer && !use_second_buf)
    new_surf_template.data = display_buf_base + get_display_page_size();

  // set h/v repeat
  new_v_repeat = DPI_MODE_V_ACTIVE_LINES / new_surf_template.bounds.h;

  // check if we're actually changing scale
  if(new_v_repeat == v_repeat && new_surf_template.bounds.w == line_width)
    return;

  // don't do it yet if already started
  // (will set need_mode_change after next render)
  if(started)
    return;

  v_repeat = new_v_repeat;
  line_width = new_surf_template.bounds.w;

  update_h_repeat();

  // reconfigure DMA channels
  // FIXME: update addr for 2nd+ line
  for(int i = 0; i < DPI_NUM_DMA_CHANNELS; i++)
    dma_channel_set_trans_count(DPI_DMA_CH_BASE + i, line_width / 2, false);
}

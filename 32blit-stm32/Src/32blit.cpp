#include "string.h"
#include <map>
#include <bitset>

#include "32blit.h"
#include "main.h"

#include "sound.hpp"
#include "display.hpp"
#include "gpio.hpp"
#include "file.hpp"
#include "jpeg.hpp"
#include "executable.hpp"

#include "adc.h"
#include "tim.h"
#include "rng.h"
#include "spi.h"
#include "i2c.h"
#include "i2c-msa301.h"
#include "i2c-lis3dh.h"
#include "i2c-bq24295.h"
#include "fatfs.h"
#include "quadspi.h"
#include "usbd_core.h"

#include "32blit.hpp"
#include "engine/api_private.hpp"
#include "graphics/color.hpp"
#include "engine/running_average.hpp"
#include "engine/menu.hpp"

#include "stdarg.h"
using namespace blit;

extern char __ltdc_start;
extern char __fb_start;
extern char itcm_text_start;
extern char itcm_text_end;
extern char itcm_data;
extern USBD_HandleTypeDef hUsbDeviceHS;

#define ADC_BUFFER_SIZE 32

__attribute__((section(".dma_data"))) ALIGN_32BYTES(__IO uint16_t adc1data[ADC_BUFFER_SIZE]);
__attribute__((section(".dma_data"))) ALIGN_32BYTES(__IO uint16_t adc3data[ADC_BUFFER_SIZE]);

FATFS filesystem;
extern Disk_drvTypeDef disk;
static bool fs_mounted = false;

bool needs_render = true;
bool exit_game = false;
bool take_screenshot = false;
uint32_t flip_cycle_count = 0;
float volume_log_base = 2.0f;
RunningAverage<float> battery_average(8);
float battery = 0.0f;
uint8_t battery_status = 0;
uint8_t battery_fault = 0;
uint16_t accel_address = LIS3DH_DEVICE_ADDRESS;

uint16_t charge_led_counter = 0;
uint8_t charge_led_r = 0;
uint8_t charge_led_g = 0;
uint8_t charge_led_b = 0;

const uint32_t long_press_exit_time = 1000;

__attribute__((section(".persist"))) Persist persist;

static bool (*do_tick)(uint32_t time) = blit::tick;

// pointers to user code
static bool (*user_tick)(uint32_t time) = nullptr;
static void (*user_render)(uint32_t time) = nullptr;

void DFUBoot(void)
{
  // Set the special magic word value that's checked by the assembly entry Point upon boot
  // This will trigger a jump into DFU mode upon reboot
  *((uint32_t *)0x2001FFFC) = 0xCAFEBABE; // Special Key to End-of-RAM

  SCB_CleanDCache();
  NVIC_SystemReset();
}

static void init_api_shared() {
  // Reset button state, this prevents the user app immediately seeing the last button transition used to launch the game
  api.buttons.state = 0;
  api.buttons.pressed = 0;
  api.buttons.released = 0;

  // reset shared outputs
  api.vibration = 0.0f;
  api.LED = Pen();
}

int blit_debugf(const char * psFormatString, va_list args)
{
	return vprintf(psFormatString, args);
}

void blit_debug(std::string message) {
	printf(message.c_str());
  screen.pen = Pen(255, 255, 255);
  screen.text(message, minimal_font, Point(0, 0));
}

void blit_exit(bool is_error) {
  if(is_error)
    blit_reset_with_error(); // likely an abort
  else
    blit_switch_execution(0); // switch back to firmware
}

void enable_us_timer()
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t get_us_timer()
{
	uint32_t uTicksPerUs = SystemCoreClock / 1000000;
	return DWT->CYCCNT/uTicksPerUs;
}

uint32_t get_max_us_timer()
{
	uint32_t uTicksPerUs = SystemCoreClock / 1000000;
	return UINT32_MAX / uTicksPerUs;
}

const char *battery_vbus_status() {
  switch(battery_status >> 6){
    case 0b00: // Unknown
      return "Unknown";
    case 0b01: // USB Host
      return "USB Host";
    case 0b10: // Adapter Port
      return "Adapter";
    case 0b11: // OTG
      return "OTG";
  }

  // unreachable
  return "";
}

const char *battery_charge_status() {
  switch((battery_status >> 4) & 0b11){
    case 0b00: // Not Charging
      return "Nope";
    case 0b01: // Pre-charge
      return "Pre";
    case 0b10: // Fast Charging
      return "Fast";
    case 0b11: // Charge Done
      return "Done";
  }

  // unreachable
  return "";
}

static void do_render() {
  if(display::needs_render) {
    blit::render(blit::now());
    display::enable_vblank_interrupt();
  }
}

void render_yield() {
  do_render();
}

void blit_tick() {
  if(exit_game) {
    if(blit_user_code_running()) {
      blit::LED.r = 0;
      blit_switch_execution(0);
    }
  }

  do_render();

  blit_i2c_tick();
  blit_process_input();
  blit_update_led();
  blit_update_vibration();

  // SD card inserted/removed
  if(blit_sd_detected() != fs_mounted) {
    if(!fs_mounted)
      fs_mounted = f_mount(&filesystem, "", 1) == FR_OK;
    else
      fs_mounted = false;

    disk.is_initialized[0] = fs_mounted; // this gets set without checking if the init succeeded, un-set it if the init failed (or the card was removed)
  }

  do_tick(blit::now());
}

bool blit_sd_detected() {
  return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11) == 1;
}

bool blit_sd_mounted() {
  return fs_mounted;
}

void hook_render(uint32_t time) {
  /*
  Replace blit::render = ::render; with blit::render = hook_render;
  and do silly on-screen debug stuff here for the great justice.
  */
  ::render(time);

  blit::screen.pen = Pen(255, 255, 255);
  for(auto i = 0; i < ADC_BUFFER_SIZE; i++) {
    int x = i / 8;
    int y = i % 8;
    blit::screen.text(std::to_string(adc1data[i]), minimal_font, Point(x * 30, y * 10));
  }
}

enum I2CState {
  DELAY,
  STOPPED,

  SEND_ACL,
  RECV_ACL,
  PROC_ACL,

  SEND_BAT,
  RECV_BAT,
  PROC_BAT
};

RunningAverage<float> accel_x(8);
RunningAverage<float> accel_y(8);
RunningAverage<float> accel_z(8);

I2CState i2c_state = SEND_ACL;
uint8_t i2c_buffer[6] = {0};
uint8_t i2c_reg = 0;
HAL_StatusTypeDef i2c_status = HAL_OK;
uint32_t i2c_delay_until = 0;
I2CState i2c_next_state = SEND_ACL;

void blit_i2c_delay(uint16_t ms, I2CState state) {
  i2c_delay_until = HAL_GetTick() + ms;
  i2c_next_state = state;
  i2c_state = DELAY;
}

void blit_i2c_tick() {
  if(i2c_state == STOPPED) {
    return;
  }
  if(i2c_state == DELAY) {
    if(HAL_GetTick() >= i2c_delay_until){
      i2c_state = i2c_next_state;
    }
  }
  if(HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY){
    return;
  }

  switch(i2c_state) {
    case STOPPED:
    case DELAY:
      break;
    case SEND_ACL:
      i2c_reg = is_beta_unit ? MSA301_X_ACCEL_RESISTER : (LIS3DH_OUT_X_L | LIS3DH_ADDR_AUTO_INC);
      i2c_status = HAL_I2C_Master_Transmit_IT(&hi2c4, accel_address, &i2c_reg, 1);
      if(i2c_status == HAL_OK){
        i2c_state = RECV_ACL;
      } else {
        blit_i2c_delay(16, SEND_ACL);
      }
      break;
    case RECV_ACL:
      i2c_status = HAL_I2C_Master_Receive_IT(&hi2c4, accel_address, i2c_buffer, 6);
      if(i2c_status == HAL_OK){
        i2c_state = PROC_ACL;
      } else {
        blit_i2c_delay(16, SEND_ACL);
      }
      break;
    case PROC_ACL:
      // LIS3DH & MSA301 - 12-bit left-justified
      accel_x.add(((int8_t)i2c_buffer[1] << 6) | (i2c_buffer[0] >> 2));
      accel_y.add(((int8_t)i2c_buffer[3] << 6) | (i2c_buffer[2] >> 2));
      accel_z.add(((int8_t)i2c_buffer[5] << 6) | (i2c_buffer[4] >> 2));

      if(is_beta_unit){
        blit::tilt = Vec3(
          accel_x.average(),
          accel_y.average(),
          -accel_z.average()
        );
      } else {
        blit::tilt = Vec3(
          -accel_x.average(),
          -accel_y.average(),
          -accel_z.average()
        );
      }

      blit::tilt.normalize();

      i2c_state = SEND_BAT;
      break;
    case SEND_BAT:
      i2c_reg = BQ24295_SYS_STATUS_REGISTER;
      HAL_I2C_Master_Transmit_IT(&hi2c4, BQ24295_DEVICE_ADDRESS, &i2c_reg, 1);
      i2c_state = RECV_BAT;
      break;
    case RECV_BAT:
      HAL_I2C_Master_Receive_IT(&hi2c4, BQ24295_DEVICE_ADDRESS, i2c_buffer, 2);
      i2c_state = PROC_BAT;
      break;
    case PROC_BAT:
      battery_status = i2c_buffer[0];
      battery_fault = i2c_buffer[1];
      blit_i2c_delay(16, SEND_ACL);
      break;
  }
}

void blit_update_volume() {
    blit::volume = (uint16_t)(65535.0f * log(1.0f + (volume_log_base - 1.0f) * persist.volume) / log(volume_log_base));
}

static void save_screenshot() {
  int index = 0;
  char buf[100];

  do {
    snprintf(buf, 100, "screenshot%i.bmp", index);

    if(!::file_exists(buf))
      break;

    index++;
  } while(true);

  screen.save(buf);
}

void blit_init() {
    // enable backup sram
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_BKPRAM_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess(); 
    HAL_PWREx_EnableBkUpReg();

    // need to wit for sram, I tried a few things I found on the net to wait
    // based on PWR flags but none seemed to work, a simple delay does work!
    HAL_Delay(5);
      
    if(persist.magic_word != persistence_magic_word) {
      // Set persistent defaults if the magic word does not match
      persist.magic_word = persistence_magic_word;
      persist.volume = 0.5f;
      persist.backlight = 1.0f;
      persist.selected_menu_item = 0;
      persist.reset_target = prtFirmware;
      persist.reset_error = false;
      persist.last_game_offset = 0;
    }

    init_api_shared();

    blit_update_volume();

    HAL_TIM_Base_Start_IT(&htim16);

    // enable cycle counting
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1data, ADC_BUFFER_SIZE);
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *)adc3data, ADC_BUFFER_SIZE);

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    fs_mounted = f_mount(&filesystem, "", 1) == FR_OK;  // this shouldn't be necessary here right?

    if(is_beta_unit){
      msa301_init(&hi2c4, MSA301_CONTROL2_POWR_MODE_NORMAL, 0x00, MSA301_CONTROL1_ODR_62HZ5);
      accel_address = MSA301_DEVICE_ADDRESS;
    } else {
      lis3dh_init(&hi2c4);
    }
    bq24295_init(&hi2c4);
    blit::api.debug = blit_debug;
    blit::api.debugf = blit_debugf;
    blit::api.now = HAL_GetTick;
    blit::api.random = HAL_GetRandom;
    blit::api.exit = blit_exit;

    blit::api.set_screen_mode = display::set_screen_mode;
    blit::api.set_screen_palette = display::set_screen_palette;
    display::set_screen_mode(blit::lores);
    blit::update = ::update;
    blit::render = ::render;
    blit::init   = ::init;
    blit::api.open_file = ::open_file;
    blit::api.read_file = ::read_file;
    blit::api.write_file = ::write_file;
    blit::api.close_file = ::close_file;
    blit::api.get_file_length = ::get_file_length;
    blit::api.list_files = ::list_files;
    blit::api.file_exists = ::file_exists;
    blit::api.directory_exists = ::directory_exists;
    blit::api.create_directory = ::create_directory;
    blit::api.rename_file = ::rename_file;
    blit::api.remove_file = ::remove_file;

    blit::api.enable_us_timer = ::enable_us_timer;
    blit::api.get_us_timer = ::get_us_timer;
    blit::api.get_max_us_timer = ::get_max_us_timer;

    blit::api.decode_jpeg_buffer = blit_decode_jpeg_buffer;
    blit::api.decode_jpeg_file = blit_decode_jpeg_file;


  display::init();
  
  blit::init();

}

/*

    Menu Items

*/

enum MenuItem {
    BACKLIGHT,
    VOLUME,
    SCREENSHOT,
    DFU,
    SHIPPING,
    SWITCH_EXE,
    LAST_COUNT // leave me last pls
};

static const Pen menu_colours[]{
  {0}, 
  { 30,  30,  50, 200}, // background
  {255, 255, 255}, // foreground
  { 40,  40,  60}, // bar background
  { 50,  50,  70}, // selected item background
  {255, 128,   0}, // battery unknown
  {  0, 255,   0}, // battery usb host/adapter port
  {255,   0,   0}, // battery otg
  {100, 100, 255}, // battery charging
  {235, 245, 255}, // header/footer bg
  {  3,   5,   7}, // header/footer fg
};
static constexpr int num_menu_colours = sizeof(menu_colours) / sizeof(Pen);
static Pen menu_saved_colours[num_menu_colours];

static Pen get_menu_colour(int index){return screen.format == PixelFormat::P ? Pen(index) : menu_colours[index];};

class FirmwareMenu final : public Menu {
public:
  using Menu::Menu;

  void prepare() {
    // slightly nasty dynamic label
    const_cast<Item *>(items)[SWITCH_EXE].label = blit_user_code_running() ? "Exit Game" :  "Launch Game";

    background_colour = get_menu_colour(1);
    foreground_colour = get_menu_colour(2);
    bar_background_color = get_menu_colour(3);
    selected_item_background = get_menu_colour(4);
    header_background = get_menu_colour(9);
    header_foreground = get_menu_colour(10);

    display_rect.w = screen.bounds.w;
    display_rect.h = screen.bounds.h;
  }

  void draw_slider(Point pos, int width, float value, Pen colour) const {
    const int bar_margin = 2;
    const int bar_height = item_h - bar_margin * 2;

    screen.pen = bar_background_color;
    screen.rectangle(Rect(pos, Size(width, bar_height)));
    screen.pen = colour;
    screen.rectangle(Rect(pos, Size(width * value, bar_height)));
  }

protected:
  void render_item(const Item &item, int y, int index) const override {
    Menu::render_item(item, y, index);

    const auto screen_width = screen.bounds.w;

    const int bar_margin = 2;
    const int bar_height = item_h - bar_margin * 2;
    const int bar_width = 75;
    int bar_x = screen_width - bar_width - item_padding_x;
  
    switch(item.id) {
      case BACKLIGHT:
        draw_slider(Point(bar_x, y + bar_margin), bar_width, persist.backlight, foreground_colour);
        break;
      case VOLUME:
        draw_slider(Point(bar_x, y + bar_margin), bar_width, persist.volume, foreground_colour);
        break;
      default:
        screen.pen = foreground_colour;
        screen.text("Press A", minimal_font, Point(screen_width - item_padding_x, y + 1), true, TextAlign::right);
        break;  
    }
  }

  void update_item(const Item &item) override {
    if(item.id == BACKLIGHT) {
      if (blit::buttons & blit::Button::DPAD_LEFT) {
        persist.backlight -= 1.0f / 256.0f;
      } else if (blit::buttons & blit::Button::DPAD_RIGHT) {
        persist.backlight += 1.0f / 256.0f;
      }
      persist.backlight = std::fmin(1.0f, std::fmax(0.0f, persist.backlight));
    } else if(item.id == VOLUME) {
      if (blit::buttons & blit::Button::DPAD_LEFT) {
        persist.volume -= 1.0f / 256.0f;
      } else if (blit::buttons & blit::Button::DPAD_RIGHT) {
        persist.volume += 1.0f / 256.0f;
      }
      persist.volume = std::fmin(1.0f, std::fmax(0.0f, persist.volume));
      blit_update_volume();
    }
  }

  void item_activated(const Item &item) override {
    switch(item.id) {
      case SCREENSHOT:
        take_screenshot = true;
        break;
      case DFU:
        DFUBoot();
        break;
      case SHIPPING:
        bq24295_enable_shipping_mode(&hi2c4);
        break;
      case SWITCH_EXE:
        blit_switch_execution(persist.last_game_offset);
        break;
    }
  }

  Pen bar_background_color;
};

static Menu::Item firmware_menu_items[]{
  {BACKLIGHT, "Backlight"},
  {VOLUME, "Volume"},
  {SCREENSHOT, "Take Screenshot"},
  {DFU, "DFU Mode"},
  {SHIPPING, "Power Off"},
  {SWITCH_EXE, ""} // label depends on if a game is running
};

FirmwareMenu firmware_menu("System Menu", firmware_menu_items, MenuItem::LAST_COUNT);

void blit_menu_update(uint32_t time) {
  firmware_menu.update(time);
}

void blit_menu_render(uint32_t time) {

  if(user_render)
    user_render(time);
  else
    ::render(time);

  // save screenshot before we render the menu over it
  if(take_screenshot) {
    // restore game colours
    if(screen.format == PixelFormat::P)
      set_screen_palette(menu_saved_colours, num_menu_colours);
  
    save_screenshot();
    take_screenshot = false;

    if(screen.format == PixelFormat::P)
      set_screen_palette(menu_colours, num_menu_colours);
  }

  firmware_menu.render();

  const int screen_width = blit::screen.bounds.w;
  const int screen_height = blit::screen.bounds.h;

  const Pen foreground_colour = get_menu_colour(10);
  const Pen bar_background_color = get_menu_colour(3);

  screen.pen = foreground_colour;

  // footer content
  char buf[100];
  snprintf(buf, 100, "Charge: %s   VBus: %s   Voltage: %i.%iv",
    battery_charge_status(),
    battery_vbus_status(),
    int(battery), int((battery - int(battery)) * 10.0f));
  screen.text(buf, minimal_font, Point(5, screen_height - 11));

  /*
  // Raw register values can be displayed with a fixed-width font using std::bitset<8> for debugging
  screen.text(
    "Fault: " + std::bitset<8>(battery_fault).to_string() +
    " Status: " + std::bitset<8>(battery_status).to_string(),
    minimal_font, Point(0, screen_height - 10), false);
  */

  // add battery info to header
  screen.text("bat", minimal_font, Point(screen_width - 80, 4));
  int battery_meter_width = 55;
  battery_meter_width = float(battery_meter_width) * (battery - 3.0f) / 1.1f;
  battery_meter_width = std::max(0, std::min(55, battery_meter_width));

  screen.pen = bar_background_color;
  screen.rectangle(Rect(screen_width - 60, 5, 55, 5));

  switch(battery_status >> 6){
    case 0b00: // Unknown
        screen.pen = get_menu_colour(5);
        break;
    case 0b01: // USB Host
        screen.pen = get_menu_colour(6);
        break;
    case 0b10: // Adapter Port
        screen.pen = get_menu_colour(6);
        break;
    case 0b11: // OTG
        screen.pen = get_menu_colour(7);
        break;
  }
  screen.rectangle(Rect(screen_width - 60, 5, battery_meter_width, 5));
  uint8_t battery_charge_status = (battery_status >> 4) & 0b11;
  if(battery_charge_status == 0b01 || battery_charge_status == 0b10){
    int battery_fill_width = (time / 500) % battery_meter_width;
    battery_fill_width = std::min(battery_meter_width, battery_fill_width);
    screen.pen = get_menu_colour(8);
    screen.rectangle(Rect(screen_width - 60, 5, battery_fill_width, 5));
  }
}

void blit_menu() {
  if(blit::update == blit_menu_update && do_tick == blit::tick) {
    if (user_tick) {
      // user code was running
      do_tick = user_tick;
      blit::render = user_render;
    } else {
      blit::update = ::update;
      blit::render = ::render;
    }

    // restore game colours
    if(screen.format == PixelFormat::P)
      set_screen_palette(menu_saved_colours, num_menu_colours);

  }
  else
  {
    firmware_menu.prepare();

    blit::update = blit_menu_update;
    blit::render = blit_menu_render;
    do_tick = blit::tick;

    if(screen.format == PixelFormat::P) {
      memcpy(menu_saved_colours, screen.palette, num_menu_colours * sizeof(Pen));
      set_screen_palette(menu_colours, num_menu_colours);
    }
  }
}

void blit_update_vibration() {
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, blit::vibration * 2000.0f);
}

void blit_update_led() {
    // RED Led
    float compare_r = (blit::LED.r * 10000) / 255;
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, compare_r);

    // GREEN Led
    float compare_g = (blit::LED.g * 10000) / 255;
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_4, compare_g);
  
    // BLUE Led
    float compare_b = (blit::LED.b * 10000) / 255;
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, compare_b);

    // Backlight
    __HAL_TIM_SetCompare(&htim15, TIM_CHANNEL_1, 962 - (962 * persist.backlight));


    // TODO we don't want to do this too often!
    switch((battery_status >> 4) & 0b11){
      case 0b00: // Not charging
        charge_led_r = 128;
        charge_led_b = 0;
        charge_led_g = 0;
        break;
      case 0b01: // Pre-charge
        charge_led_r = 128;
        charge_led_b = 128;
        charge_led_g = 0;
        break;
      case 0b10: // Fast Charging
        charge_led_r = 0;
        charge_led_b = 128;
        charge_led_g = 0;
        break;
      case 0b11: // Charge Done
        charge_led_r = 0;
        charge_led_b = 0;
        charge_led_g = 128;
        break;
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc){
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
  if(hadc->Instance == ADC1) {
    SCB_InvalidateDCache_by_Addr((uint32_t *) &adc1data[0], ADC_BUFFER_SIZE);
  } else if (hadc->Instance == ADC3) {
    SCB_InvalidateDCache_by_Addr((uint32_t *) &adc3data[0], ADC_BUFFER_SIZE);
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  if(hadc->Instance == ADC1) {
    SCB_InvalidateDCache_by_Addr((uint32_t *) &adc1data[ADC_BUFFER_SIZE / 2], ADC_BUFFER_SIZE / 2);
  } else if (hadc->Instance == ADC3) {
    SCB_InvalidateDCache_by_Addr((uint32_t *) &adc3data[ADC_BUFFER_SIZE / 2], ADC_BUFFER_SIZE / 2);
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if(htim == &htim2) {
    bool pressed = !HAL_GPIO_ReadPin(BUTTON_MENU_GPIO_Port, BUTTON_MENU_Pin);
    if(pressed) {
      exit_game = true;
    }
    HAL_TIM_Base_Stop(&htim2);
    HAL_TIM_Base_Stop_IT(&htim2);
  } else if(htim == &htim16) {
    charge_led_counter += 1;
    charge_led_counter &= 0x3ff;
    HAL_GPIO_WritePin(LED_CHG_RED_Port, LED_CHG_RED_Pin, charge_led_r > charge_led_counter ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_CHG_GREEN_Port, LED_CHG_GREEN_Pin, charge_led_g > charge_led_counter ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_CHG_BLUE_Port, LED_CHG_BLUE_Pin, charge_led_b > charge_led_counter ? GPIO_PIN_RESET : GPIO_PIN_SET);
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  bool pressed = !HAL_GPIO_ReadPin(BUTTON_MENU_GPIO_Port, BUTTON_MENU_Pin);
  if(pressed) {
    /*
    The timer will generate a spurious interrupt as soon as it's enabled- apparently to load the compare value.
    We disable interrupts and clear this early interrupt flag before re-enabling them so that the *real*
    interrupt can fire. 
    */
    if(!((&htim2)->Instance->CR1 & TIM_CR1_CEN)){
      HAL_NVIC_DisableIRQ(TIM2_IRQn);
      __HAL_TIM_SetCounter(&htim2, 0);
      __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, long_press_exit_time * 10); // press-to-reset-time
      HAL_TIM_Base_Start(&htim2);
      HAL_TIM_Base_Start_IT(&htim2);
      __HAL_TIM_CLEAR_FLAG(&htim2, TIM_SR_UIF);
      HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }

  } else {
    if(__HAL_TIM_GetCounter(&htim2) > 200){ // 20ms debounce time
      // TODO is it a good idea to swap out the render/update functions potentially in the middle of a loop?
      // We were more or less doing this before by handling the menu update between render/update so perhaps it's mostly fine.
      blit_menu();
      HAL_TIM_Base_Stop(&htim2);
      HAL_TIM_Base_Stop_IT(&htim2);
      __HAL_TIM_SetCounter(&htim2, 0);
    }
  }
}

#define ACCEL_OVER_SAMPLE 16

uint8_t tilt_sample_offset = 0;
int16_t acceleration_data_buffer[3 * ACCEL_OVER_SAMPLE] = {0};

void blit_disable_ADC()
{
  // TODO: Flesh this out if it's still necessary in interrupt-driven ADC mode
  return;
}

void blit_enable_ADC()
{
  // TODO: Flesh this out if it's still necessary in interrupt-driven ADC mode
  return;
}

void blit_process_input() {
  bool joystick_button = false;

  // Read buttons
  blit::buttons =
    (!HAL_GPIO_ReadPin(DPAD_UP_GPIO_Port,     DPAD_UP_Pin)      ? blit::DPAD_UP    : 0) |
    (!HAL_GPIO_ReadPin(DPAD_DOWN_GPIO_Port,   DPAD_DOWN_Pin)    ? blit::DPAD_DOWN  : 0) |
    (!HAL_GPIO_ReadPin(DPAD_LEFT_GPIO_Port,   DPAD_LEFT_Pin)    ? blit::DPAD_LEFT  : 0) |
    (!HAL_GPIO_ReadPin(DPAD_RIGHT_GPIO_Port,  DPAD_RIGHT_Pin)   ? blit::DPAD_RIGHT : 0) |
    (!HAL_GPIO_ReadPin(BUTTON_A_GPIO_Port,    BUTTON_A_Pin)     ? blit::A          : 0) |
    (!HAL_GPIO_ReadPin(BUTTON_B_GPIO_Port,    BUTTON_B_Pin)     ? blit::B          : 0) |
    (!HAL_GPIO_ReadPin(BUTTON_X_GPIO_Port,    BUTTON_X_Pin)     ? blit::X          : 0) |
    (!HAL_GPIO_ReadPin(BUTTON_Y_GPIO_Port,    BUTTON_Y_Pin)     ? blit::Y          : 0) |
    (HAL_GPIO_ReadPin(BUTTON_HOME_GPIO_Port,  BUTTON_HOME_Pin)  ? blit::HOME       : 0) |  // INVERTED LOGIC!
    (!HAL_GPIO_ReadPin(BUTTON_MENU_GPIO_Port, BUTTON_MENU_Pin)  ? blit::MENU       : 0) |
    (!HAL_GPIO_ReadPin(JOYSTICK_BUTTON_GPIO_Port, JOYSTICK_BUTTON_Pin) ? blit::JOYSTICK   : 0);

  // Process ADC readings
  int joystick_x = (adc1data[0] >> 1) - 16384;
  joystick_x = std::max(-8192, std::min(8192, joystick_x));
  if(joystick_x < -1024) {
    joystick_x += 1024;
  }
  else if(joystick_x > 1024) {
    joystick_x -= 1024;
  } else {
    joystick_x = 0;
  }
  blit::joystick.x = joystick_x / 7168.0f;

  int joystick_y = (adc1data[1] >> 1) - 16384;
  joystick_y = std::max(-8192, std::min(8192, joystick_y));
  if(joystick_y < -1024) {
    joystick_y += 1024;
  }
  else if(joystick_y > 1024) {
    joystick_y -= 1024;
  } else {
    joystick_y = 0;
  }
  blit::joystick.y = -joystick_y / 7168.0f;

  blit::hack_left = (adc3data[0] >> 1) / 32768.0f;
  blit::hack_right = (adc3data[1] >> 1)  / 32768.0f;

  battery_average.add(6.6f * adc3data[2] / 65535.0f);

  battery = battery_average.average();
}

char *get_fr_err_text(FRESULT err){
  switch(err){
    case FR_OK:
      return "OK";
    case FR_DISK_ERR:
      return "DISK_ERR";
    case FR_INT_ERR:
      return "INT_ERR";
    case FR_NOT_READY:
      return "NOT_READY";
    case FR_NO_FILE:
      return "NO_FILE";
    case FR_NO_PATH:
      return "NO_PATH";
    case FR_INVALID_NAME:
      return "INVALID_NAME";
    case FR_DENIED:
      return "DENIED";
    case FR_EXIST:
      return "EXIST";
    case FR_INVALID_OBJECT:
      return "INVALID_OBJECT";
    case FR_WRITE_PROTECTED:
      return "WRITE_PROTECTED";
    case FR_INVALID_DRIVE:
      return "INVALID_DRIVE";
    case FR_NOT_ENABLED:
      return "NOT_ENABLED";
    case FR_NO_FILESYSTEM:
      return "NO_FILESYSTEM";
    case FR_MKFS_ABORTED:
      return "MKFS_ABORTED";
    case FR_TIMEOUT:
      return "TIMEOUT";
    case FR_LOCKED:
      return "LOCKED";
    case FR_NOT_ENOUGH_CORE:
      return "NOT_ENOUGH_CORE";
    case FR_TOO_MANY_OPEN_FILES:
      return "TOO_MANY_OPEN_FILES";
    case FR_INVALID_PARAMETER:
      return "INVALID_PARAMETER";
    default:
      return "INVALID_ERR_CODE";
  }
}



// blit_switch_execution
//
// Switches execution to new location defined by EXTERNAL_LOAD_ADDRESS
// EXTERNAL_LOAD_ADDRESS is the start of the Vector Table location
//
typedef  void (*pFunction)(void);
pFunction JumpToApplication;

void blit_switch_execution(uint32_t address)
{
  if(blit_user_code_running())
    persist.reset_target = prtFirmware;
  else
    persist.reset_target = prtGame;

  init_api_shared();

  // returning from game running on top of the firmware
  if(user_tick) {
    user_tick = nullptr;
    user_render = nullptr;
    blit::render = ::render;
    blit::update = ::update;
    do_tick = blit::tick;

    // TODO: may be possible to return to the menu without a hard reset but currently flashing doesn't work
    SCB_CleanDCache();
    NVIC_SystemReset();
  }

	// switch to user app in external flash
	if(EXTERNAL_LOAD_ADDRESS >= 0x90000000) {
		qspi_enable_memorymapped_mode();

    auto game_header = ((__IO BlitGameHeader *) (EXTERNAL_LOAD_ADDRESS + address));

    if(game_header->magic == blit_game_magic) {
      // load function pointers
      auto init = (BlitInitFunction)((uint8_t *)game_header->init);
      if(!init()) {
        // this would just be a return, but qspi is already mapped by this point
        persist.reset_target = prtFirmware;
        SCB_CleanDCache();
        NVIC_SystemReset();
        return;
      }
  
      persist.last_game_offset = address;

      blit::render = user_render = (BlitRenderFunction) ((uint8_t *)game_header->render);
      do_tick = user_tick = (BlitTickFunction) ((uint8_t *)game_header->tick);
      return;
    }
    // anything flashed at a non-zero offset should have a valid header
    else if(address != 0)
      return;
  }

  // old-style soft-reset to app with linked HAL
  // left for compatibility/testing

  // Stop the ADC DMA
  HAL_ADC_Stop_DMA(&hadc1);
  HAL_ADC_Stop_DMA(&hadc3);

  // Stop the audio
  HAL_TIM_Base_Stop_IT(&htim6);
  HAL_DAC_Stop(&hdac1, DAC_CHANNEL_2);

  // Stop system button timer
  HAL_TIM_Base_Stop_IT(&htim2);
  HAL_TIM_Base_Stop_IT(&htim16);

  // stop USB
  USBD_Stop(&hUsbDeviceHS);
  
  // Disable all the interrupts... just to be sure
  HAL_NVIC_DisableIRQ(LTDC_IRQn);
  HAL_NVIC_DisableIRQ(ADC_IRQn);
  HAL_NVIC_DisableIRQ(ADC3_IRQn);
  HAL_NVIC_DisableIRQ(DMA1_Stream0_IRQn);
  HAL_NVIC_DisableIRQ(DMA1_Stream1_IRQn);
  HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
  HAL_NVIC_DisableIRQ(OTG_HS_IRQn);
  HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
  HAL_NVIC_DisableIRQ(TIM2_IRQn);

	volatile uint32_t uAddr = EXTERNAL_LOAD_ADDRESS;

	/* Disable I-Cache */
	SCB_DisableICache();

	/* Disable D-Cache */
	SCB_DisableDCache();

	/* Disable Systick interrupt */
	SysTick->CTRL = 0;

	/* Initialize user application's Stack Pointer & Jump to user application */
	JumpToApplication = (pFunction) (*(__IO uint32_t*) (EXTERNAL_LOAD_ADDRESS + 4));
	__set_MSP(*(__IO uint32_t*) EXTERNAL_LOAD_ADDRESS);
	JumpToApplication();

	/* We should never get here as execution is now on user application */
	while(1)
	{
	}
}

bool blit_user_code_running() {
  // running fully linked code from ext flash
  if(APPLICATION_VTOR == 0x90000000)
    return true;

  // loaded user-only game from flash
  return user_tick != nullptr;
}

void blit_reset_with_error() {
  persist.reset_error = true;
  SCB_CleanDCache();
  NVIC_SystemReset();
}

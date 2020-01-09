#include "string.h"

#include "32blit.h"
#include "main.h"
#include "adc.h"
#include "ltdc.h"
#include "dac.h"
#include "tim.h"
#include "rng.h"
#include "spi.h"
#include "spi-st7272a.h"
#include "i2c.h"
#include "i2c-msa301.h"
#include "i2c-bq24295.h"
#include "fatfs.h"

#include "32blit.hpp"
#include "graphics/color.hpp"

#include "stdarg.h"
using namespace blit;

__attribute__((section(".dac_data"))) uint16_t dac_buffer[DAC_BUFFER_SIZE];

extern char __ltdc_start;
extern char itcm_text_start;
extern char itcm_text_end;
extern char itcm_data;

FATFS filesystem;
FRESULT SD_Error = FR_INVALID_PARAMETER;
FRESULT SD_FileOpenError = FR_INVALID_PARAMETER;

uint32_t total_samples = 0;
uint8_t dma_status = 0;
uint8_t global_volume = 255;

static blit::screen_mode mode = blit::screen_mode::lores;

/* configure the screen surface to point at the reserved LTDC framebuffer */
surface __ltdc((uint8_t *)&__ltdc_start, pixel_format::RGB565, size(320, 240));
uint8_t ltdc_buffer_id = 0;

surface __fb(((uint8_t *)&__ltdc_start) + (320 * 240 * 2), pixel_format::RGB, size(160, 120));

void DFUBoot(void)
{
  // Set the special magic word value that's checked by the assembly entry point upon boot
  // This will trigger a jump into DFU mode upon reboot
  *((uint32_t *)0x2001FFFC) = 0xCAFEBABE; // Special Key to End-of-RAM

  SCB_CleanDCache();
  NVIC_SystemReset();
}

int blit_debugf(const char * psFormatString, ...)
{
	va_list args;
	va_start(args, psFormatString);
	int ret = vprintf(psFormatString, args);
	va_end(args);
	return ret;
}
void blit_debug(std::string message) {
	printf(message.c_str());
    fb.pen(rgba(255, 255, 255));
    fb.text(message, &minimal_font[0][0], point(0, 0));
}

/*void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef *hdac){
  dma_status = DAC_DMA_HALF_COMPLETE;
}

void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef *hdac){
  dma_status = DAC_DMA_COMPLETE;
}*/


uint32_t audio_tick_cycle_count = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if(htim->Instance == TIM6) {    
    uint32_t scc = DWT->CYCCNT;
    hdac1.Instance->DHR12R2 = blit::audio::get_audio_frame() >> 4;
    audio_tick_cycle_count = DWT->CYCCNT - scc;


  }
}



/*
uint32_t blit_update_dac(FIL *audio_file) {
  uint16_t buffer_offset = 0;
  unsigned int read = 0;
  uint8_t buf[DAC_BUFFER_SIZE / 2] = {0};

  if(dma_status){
    if(dma_status == DAC_DMA_COMPLETE){
      buffer_offset = (DAC_BUFFER_SIZE / 2);
    }
    dma_status = 0;
    FRESULT err = f_read(audio_file, buf, DAC_BUFFER_SIZE / 2, &read);

    if(err == FR_OK){
      for(unsigned int x = 0; x < read; x++){
        dac_buffer[x + buffer_offset] = buf[x] * 16.0f * blit::volume;
      }
      if(read < DAC_BUFFER_SIZE / 2){
        // If we have a short read, seek back to 0 in our audio file
        // and fill the rest of the DMA buffer with zeros cos it's
        // slightly easier than filling it with data.
        f_lseek(audio_file, 0);
        for(unsigned int x = 0; x < (DAC_BUFFER_SIZE / 2) - read; x++){
          dac_buffer[x + buffer_offset] = 0;
        }
      }
    }
  }

  return read;
}*/

void blit_tick() {
  blit_process_input();
  blit_update_led();
  blit_update_vibration();

 // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);

  if(blit::tick(blit::now())){

    fb.text(std::to_string(audio_tick_cycle_count), &minimal_font[0][0], point(0, 20));
    blit_flip();
  }
}

bool blit_sd_detected() {
  return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11) == 1;
}

bool blit_mount_sd(char label[12], uint32_t &totalspace, uint32_t &freespace) {
  DWORD free_clusters;
  FATFS *pfs;
  if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11) == 1){
    SD_Error = f_mount(&filesystem, "", 1);
    if(SD_Error == FR_OK){
      f_getlabel("", label, 0);
      f_getfree("", &free_clusters, &pfs);
      totalspace = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
      freespace = (uint32_t)(free_clusters * pfs->csize * 0.5);
      return true;
    }
  }
  return false;
}

bool blit_open_file(FIL &file, const char *filename) {
  SD_FileOpenError = f_open(&file, filename, FA_READ);
  if(SD_FileOpenError == FR_OK){
    uint8_t buf[10];
    unsigned int read;
    SD_FileOpenError = f_read(&file, buf, 10, &read);
    f_lseek(&file, 0);
    return true;
  }
  return false;
}

void blit_enable_amp() {
  HAL_GPIO_WritePin(AMP_SHUTDOWN_GPIO_Port, AMP_SHUTDOWN_Pin, GPIO_PIN_SET);
}

void blit_global_volume(uint32_t v) {
  global_volume = v;
}

void blit_init() {
    for(int x = 0; x<DAC_BUFFER_SIZE; x++){
      dac_buffer[x] = 0;
    }
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
    //HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (uint32_t*)dac_buffer, DAC_BUFFER_SIZE, DAC_ALIGN_12B_R);
    
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    ST7272A_RESET();

    st7272a_set_bgr();

    msa301_init(&hi2c4, MSA301_CONTROL2_POWR_MODE_NORMAL, 0x00, MSA301_CONTROL1_ODR_62HZ5);
    bq24295_init(&hi2c4);
    blit::backlight = 1.0f;
    blit::debug = blit_debug;
    blit::debugf = blit_debugf;
    blit::now = HAL_GetTick;
    blit::random = HAL_GetRandom;
    blit::audio::volume = 0xffff;
    blit::set_screen_mode = ::set_screen_mode;
    ::set_screen_mode(blit::lores);

    blit::update = ::update;
    blit::render = ::render;
    blit::init   = ::init;

    blit_enable_amp();

    blit::init();
}

int menu_item = 0;

void blit_menu_update(uint32_t time) {
  static uint32_t last_buttons = 0;
  uint32_t changed_buttons = blit::buttons ^ last_buttons;
  if(blit::buttons & changed_buttons & blit::button::DPAD_UP) {
    menu_item -= 1;
    if(menu_item < 0){
      menu_item = 0;
    }
  } else if (blit::buttons & changed_buttons & blit::button::DPAD_DOWN) {
    menu_item += 1;
    if(menu_item > 3){
      menu_item = 3;
    }
  } else if (blit::buttons & blit::button::DPAD_RIGHT ) {
    switch(menu_item) {
      case 0: // Backlight
        blit::backlight += 1.0f / 256.0f;
        blit::backlight = std::fmin(1.0f, std::fmax(0.0f, blit::backlight));
        break;
      case 1: // Volume
        //blit::volume += 1.0f / 256.0f;
        //blit::volume = std::fmin(1.0f, std::fmax(0.0f, blit::volume));
        break;
    }
  } else if (blit::buttons & blit::button::DPAD_LEFT ) {
    switch(menu_item) {
      case 0: // Brightness
        blit::backlight -= 1.0f / 256.0f;
        blit::backlight = std::fmin(1.0f, std::fmax(0.0f, blit::backlight));
        break;
      case 1: // Volume
        //blit::volume -= 1.0f / 256.0f;
        //blit::volume = std::fmin(1.0f, std::fmax(0.0f, blit::volume));
        break;
    }
  } else if (blit::buttons & changed_buttons & blit::button::A) {
      switch(menu_item) {
        case 2:
          DFUBoot();
          break;
        case 3:
          bq24295_enable_shipping_mode(&hi2c4);
          break;
      }
  }

  last_buttons = blit::buttons;
}

void blit_menu_render(uint32_t time) {
  ::render(time);
  int screen_width = 160;
  int screen_height = 120;
  if (mode == blit::screen_mode::hires) {
    screen_width = 320;
    screen_height = 240;
  }

  const rgba bar_background_color = rgba(40, 40, 60);

  fb.pen(rgba(30, 30, 50, 200));
  fb.clear();

  fb.pen(rgba(255, 255, 255));

  fb.text("System Menu", &minimal_font[0][0], point(5, 5));

  fb.text("bat", &minimal_font[0][0], point(screen_width / 2, 5));
  uint16_t battery_meter_width = 55;
  battery_meter_width = float(battery_meter_width) * (blit::battery - 3.0f) / 1.1f;
  battery_meter_width = std::max((uint16_t)0, std::min((uint16_t)55, battery_meter_width));

  fb.pen(bar_background_color);
  fb.rectangle(rect((screen_width / 2) + 20, 6, 55, 5));

  switch(battery_vbus_status){
    case 0b00: // Unknown
        fb.pen(rgba(255, 128, 0));
        break;
    case 0b01: // USB Host
        fb.pen(rgba(0, 255, 0));
        break;
    case 0b10: // Adapter Port
        fb.pen(rgba(0, 255, 0));
        break;
    case 0b11: // OTG
        fb.pen(rgba(255, 0, 0));
        break;
  }
  fb.rectangle(rect((screen_width / 2) + 20, 6, battery_meter_width, 5));
  if(battery_charge_status == 0b01 || battery_charge_status == 0b10){
    uint16_t battery_fill_width = uint32_t(time / 100.0f) % battery_meter_width;
    battery_fill_width = std::max((uint16_t)0, std::min((uint16_t)battery_meter_width, battery_fill_width));
    fb.pen(rgba(100, 255, 200));
    fb.rectangle(rect((screen_width / 2) + 20, 6, battery_fill_width, 5));
  }

  // Horizontal Line
  fb.pen(rgba(255, 255, 255));
  fb.rectangle(rect(0, 15, screen_width, 1));

  if(menu_item == 0){
    fb.pen(rgba(50, 50, 70));
    fb.rectangle(rect(0, 19, screen_width, 9));
    fb.pen(rgba(255, 255, 255));
  }

  fb.text("Backlight", &minimal_font[0][0], point(5, 20));
  fb.pen(bar_background_color);
  fb.rectangle(rect(screen_width / 2, 21, 75, 5));
  fb.pen(rgba(255, 255, 255));
  fb.rectangle(rect(screen_width / 2, 21, 75 * blit::backlight, 5));

  if(menu_item == 1){
    fb.pen(rgba(50, 50, 70));
    fb.rectangle(rect(0, 29, screen_width, 9));
    fb.pen(rgba(255, 255, 255));
  }

/*  fb.text("Volume", &minimal_font[0][0], point(5, 30));
  fb.pen(bar_background_color);
  fb.rectangle(rect(screen_width / 2, 31, 75, 5));
  fb.pen(rgba(255, 255, 255));
  fb.rectangle(rect(screen_width / 2, 31, 75 * blit::volume, 5));*/

  if(menu_item == 2){
    fb.pen(rgba(50, 50, 70));
    fb.rectangle(rect(0, 39, screen_width, 9));
    fb.pen(rgba(255, 255, 255));
  }

  fb.text("DFU", &minimal_font[0][0], point(5, 40));
  fb.text("Press A", &minimal_font[0][0], point(screen_width / 2, 40));

  if(menu_item == 3){
    fb.pen(rgba(50, 50, 70));
    fb.rectangle(rect(0, 49, screen_width, 9));
    fb.pen(rgba(255, 255, 255));
  }

  fb.text("Shipping", &minimal_font[0][0], point(5, 50));
  fb.text("Press A", &minimal_font[0][0], point(screen_width / 2, 50));

  // Horizontal Line
  fb.pen(rgba(255, 255, 255));
  fb.rectangle(rect(0, screen_height - 15, screen_width, 1));

}

void blit_menu() {
  if(blit::update == blit_menu_update) {
    blit::update = ::update;
    blit::render = ::render;
  }
  else
  {
    blit::update = blit_menu_update;
    blit::render = blit_menu_render;
  }
}

/**
 * In low-res mode this copies the low-res RGB framebuffer into the larger RGB565 buffer, applying pixel doubling.
 * Since the LTDC display is refreshed from this high-res buffer, the low-res one can then be safely redrawn.
 * In high-res mode it simply points LTDC at the freshly drawn buffer and gives 32blit the other buffer to draw into.
 */
void blit_flip() {
    if(mode == screen_mode::hires) {
        // HIRES mode
        SCB_CleanInvalidateDCache_by_Addr((uint32_t *)blit::fb.data, 320 * 240 * 2);

        // wait until next VSYNC period
        while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS));

        // set the LTDC layer framebuffer pointer shadow register
        LTDC_Layer1->CFBAR = (uint32_t)(&__ltdc_start + (ltdc_buffer_id * 320 * 240 * 2));
        // force LTDC driver to reload shadow registers
        LTDC->SRCR = LTDC_SRCR_IMR;

        // Swap blit's output framebuffer over
        ltdc_buffer_id = ltdc_buffer_id == 0 ? 1 : 0;
        blit::fb.data = (uint8_t *)(&__ltdc_start) + (ltdc_buffer_id * 320 * 240 * 2);
    } else {
        // LORES mode

        // wait for next frame if LTDC hardware currently drawing, ensures
        // no tearing
        while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS));

        // pixel double the framebuffer to the LTDC buffer
        rgb *src = (rgb *)blit::fb.data;

        uint16_t *dest = (uint16_t *)(&__ltdc_start);
        for(uint8_t y = 0; y < 120; y++) {
            // pixel double the current row while converting from RGBA to RGB565
            for(uint8_t x = 0; x < 160; x++) {
                uint8_t r = src->r >> 3;
                uint8_t g = src->g >> 2;
                uint8_t b = src->b >> 3;
                uint16_t c = (r << 11) | (g << 5) | (b);
                *dest++ = c;
                *dest++ = c;
                src++;
            }

            // copy the previous converted row (640 bytes / 320 x 2-byte pixels)
            memcpy((uint8_t *)(dest), (uint8_t *)(dest) - 640, 640);
            dest += 320;
        }

        SCB_CleanInvalidateDCache_by_Addr((uint32_t *)&__ltdc_start, 320 * 240 * 2);

        // set the LTDC layer framebuffer pointer shadow register
        LTDC_Layer1->CFBAR = (uint32_t)(&__ltdc_start);
        // force LTDC driver to reload shadow registers
        LTDC->SRCR = LTDC_SRCR_IMR;

        // No need to swap framebuffer since we've copied from the engine's
        // 160 x 120 framebuffer to the 320 x 240 LTDC buffer
        ltdc_buffer_id = 0;
    }
}

void set_screen_mode(blit::screen_mode new_mode) {
  mode = new_mode;

  if(mode == blit::screen_mode::hires) {
    blit::fb = __ltdc;
  } else {
    blit::fb = __fb;
  }
}

void blit_clear_framebuffer() { 
  // initialise the LTDC buffer with a checkerboard pattern so it's clear
  // when it hasn't been written to yet

  uint16_t *pc = (uint16_t *)&__ltdc_start;

  // framebuffer 1
  for(uint16_t y = 0; y < 240; y++) {
    for(uint16_t x = 0; x < 320; x++) {
      *pc++ = (((x / 10) + (y / 10)) & 0b1) ?  0x7BEF : 0x38E7;
    }
  }

  // framebuffer 2
  for(uint16_t y = 0; y < 240; y++) {
    for(uint16_t x = 0; x < 320; x++) {
      *pc++ = (((x / 10) + (y / 10)) & 0b1) ?  0x38E7 : 0x7BEF;
    }
  }
}

void blit_update_vibration() {
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, vibration * 2000.0f);
}

void blit_update_led() {
    // RED Led
    float compare_r = (LED.r * 10000) / 255;
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, compare_r);

    // GREEN Led
    float compare_g = (LED.g * 10000) / 255;
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_4, compare_g);
  
    // BLUE Led
    float compare_b = (LED.b * 10000) / 255;
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, compare_b);

    // Backlight
    __HAL_TIM_SetCompare(&htim15, TIM_CHANNEL_1, 962 - (962 * blit::backlight));
}

void ADC_update_joystick_axis(ADC_HandleTypeDef *adc, float *axis){
  if (HAL_ADC_PollForConversion(adc, 1000000) == HAL_OK)
  {
    int adc_reading = (HAL_ADC_GetValue(adc) >> 1) - 16384;
    adc_reading = std::max(-8192, std::min(8192, adc_reading));
    if (adc_reading < -1024) {
      adc_reading += 1024;
    }
    else if (adc_reading > 1024) {
      adc_reading -= 1024;
    }
    else {
      adc_reading = 0;
    }
    *axis = adc_reading / 7168.0f;
  }
}

#define ACCEL_OVER_SAMPLE 16

uint8_t tilt_sample_offset = 0;
int16_t acceleration_data_buffer[3 * ACCEL_OVER_SAMPLE] = {0};

void blit_process_input() {
  static uint32_t blit_last_buttons = 0;
  // read x axis of joystick
  bool joystick_button = false;

  HAL_ADC_Start(&hadc1);
  ADC_update_joystick_axis(&hadc1, &blit::joystick.x);
  ADC_update_joystick_axis(&hadc1, &blit::joystick.y);
  blit::joystick.y = -blit::joystick.y;
  HAL_ADC_Stop(&hadc1);

  HAL_ADC_Start(&hadc3);
  ADC_update_joystick_axis(&hadc3, &blit::hack_left);
  ADC_update_joystick_axis(&hadc3, &blit::hack_right);
  if (HAL_ADC_PollForConversion(&hadc3, 1000000) == HAL_OK)
  {
    blit::battery = 6.6f * HAL_ADC_GetValue(&hadc3) / 65535.0f;
  }
  HAL_ADC_Stop(&hadc3);

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

  // Read accelerometer
  msa301_get_accel(&hi2c4, &acceleration_data_buffer[tilt_sample_offset * 3]);

  uint8_t status = bq24295_get_status(&hi2c4);
  blit::battery_vbus_status = status >> 6; // 00 - Unknown, 01 - USB Host, 10 - Adapter port, 11 - OTG
  blit::battery_charge_status = (status >> 4) & 0b11; // 00 - Not Charging, 01 - Pre-charge, 10 - Fast Charging, 11 - Charge Termination Done

  blit::battery_fault = bq24295_get_fault(&hi2c4);

  tilt_sample_offset += 1;
  if(tilt_sample_offset >= ACCEL_OVER_SAMPLE){
    tilt_sample_offset = 0;
  }

  float tilt_x = 0, tilt_y = 0, tilt_z = 0;
  for(int x = 0; x < ACCEL_OVER_SAMPLE; x++) {
    int offset = x * 3;
    tilt_x += acceleration_data_buffer[offset + 0];
    tilt_y += acceleration_data_buffer[offset + 1];
    tilt_z += acceleration_data_buffer[offset + 2];
  }

  blit::tilt = vec3(
    -(tilt_x / ACCEL_OVER_SAMPLE),
    -(tilt_y / ACCEL_OVER_SAMPLE),
    -(tilt_z / ACCEL_OVER_SAMPLE)
    );
  blit::tilt.normalize();

  if(blit::buttons & blit::MENU && !(blit_last_buttons & blit::MENU)) {
    blit_menu();
  }

  blit_last_buttons = blit::buttons;
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
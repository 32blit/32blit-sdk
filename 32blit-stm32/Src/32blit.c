#include "string.h"
#include "stdarg.h"
#include <map>

#include "32blit.h"
#include "main.h"

#include "sound.hpp"
#include "display.hpp"
#include "gpio.hpp"
#include "file.hpp"

#include "adc.h"
#include "tim.h"
#include "rng.h"
#include "spi.h"
#include "i2c.h"
#include "i2c-msa301.h"
#include "i2c-bq24295.h"
#include "fatfs.h"
#include "quadspi.h"
#include "usbd_core.h"

// since menu refactoring, some of these can probably go? ^

#include "32blit.hpp"
#include "graphics/color.hpp"

#include "engine/menu/menu.hpp"
#include "Menu/FirmwareMenusDataSource.hpp"

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
FRESULT SD_Error = FR_INVALID_PARAMETER;
FRESULT SD_FileOpenError = FR_INVALID_PARAMETER;

FirmwareMenusDataSource dataSource;
Menu fw_menu = Menu("System Menu",dataSource.menuItems());

bool needs_render = true;
uint32_t flip_cycle_count = 0;

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
  screen.pen = Pen(255, 255, 255);
  screen.text(message, minimal_font, Point(0, 0));
}

void blit_tick() {
  if(display::needs_render) {
    blit::render(blit::now());
    display::enable_vblank_interrupt();
  }

  blit_process_input();
  blit_update_led();
  blit_update_vibration();

  blit::tick(blit::now());
}

bool blit_sd_detected() {
  return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11) == 1;
}

void blit_enable_amp() {
  HAL_GPIO_WritePin(AMP_SHUTDOWN_GPIO_Port, AMP_SHUTDOWN_Pin, GPIO_PIN_SET);
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

void blit_update_volume() {
  // this is duplicated in MenusDataSource. Refactor pleaaz
    float volume_log_base = 2.0f;
    blit::volume = (uint16_t)(65535.0f * log(1.0f + (volume_log_base - 1.0f) * persist.volume) / log(volume_log_base));
}

void blit_init() {
    if(persist.magic_word != persistence_magic_word) {
      // Set persistent defaults if the magic word does not match
      persist.magic_word = persistence_magic_word;
      persist.volume = 0.5f;
      persist.backlight = 1.0f;
      persist.selected_menu_item = 0;
    }

    blit_update_volume();

    // enable cycle counting
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1data, ADC_BUFFER_SIZE);
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *)adc3data, ADC_BUFFER_SIZE);

    
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    f_mount(&filesystem, "", 1);  // this shouldn't be necessary here right?
    msa301_init(&hi2c4, MSA301_CONTROL2_POWR_MODE_NORMAL, 0x00, MSA301_CONTROL1_ODR_62HZ5);
    bq24295_init(&hi2c4);
    blit::debug = blit_debug;
    blit::debugf = blit_debugf;
    blit::now = HAL_GetTick;
    blit::random = HAL_GetRandom;
    blit::set_screen_mode = display::set_screen_mode;
    display::set_screen_mode(blit::lores);
    blit::update = ::update;
    blit::render = ::render;
    blit::init   = ::init;
    blit::open_file = ::open_file;
    blit::read_file = ::read_file;
    blit::close_file = ::close_file;
    blit::get_file_length = ::get_file_length;
    blit::list_files = ::list_files;

    blit::switch_execution = blit_switch_execution;

    blit_enable_amp();

  display::init();
  
  blit::init();

}

void blit_menu_update(uint32_t time) {
  static uint32_t last_buttons = 0;
  uint32_t changed_buttons = blit::buttons ^ last_buttons;

  if (blit::buttons & changed_buttons & blit::Button::DPAD_UP) {
    fw_menu.decrement_selection();
  } else if (blit::buttons & changed_buttons & blit::Button::DPAD_DOWN) {
    fw_menu.increment_selection();
  } 
  
  if (blit::buttons & changed_buttons & blit::Button::DPAD_LEFT) {
    fw_menu.pressed_left();
  } else if (blit::buttons & changed_buttons & blit::Button::DPAD_RIGHT) {
    fw_menu.pressed_right();
  } else  if (blit::buttons & blit::Button::DPAD_LEFT) {
    fw_menu.held_left();
  } else if (blit::buttons & blit::Button::DPAD_RIGHT) {
    fw_menu.held_right();
  }
 
  if (pressed(blit::A)) {
    fw_menu.selected();
  }

  if (pressed(blit::B)) {
    fw_menu.back_pressed();
  }

  last_buttons = blit::buttons;
}

void blit_menu_render(uint32_t time) {
  ::render(time);

  fw_menu.render(time);

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
    __HAL_TIM_SetCompare(&htim15, TIM_CHANNEL_1, 962 - (962 * persist.backlight));
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
  static uint32_t last_battery_update = 0;
  static uint32_t last_tilt_update = 0;

  uint32_t scc = DWT->CYCCNT;
  static uint32_t blit_last_buttons = 0;
  // read x axis of joystick
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

  blit::battery = 6.6f * adc3data[2] / 65535.0f;

  if(blit::now() - last_battery_update > 5000) {
    uint8_t status = bq24295_get_status(&hi2c4);
    blit::battery_vbus_status = status >> 6; // 00 - Unknown, 01 - USB Host, 10 - Adapter port, 11 - OTG
    blit::battery_charge_status = (status >> 4) & 0b11; // 00 - Not Charging, 01 - Pre-charge, 10 - Fast Charging, 11 - Charge Termination Done

    blit::battery_fault = bq24295_get_fault(&hi2c4);

    last_battery_update = blit::now();
  }

  if(blit::now() - last_tilt_update > 10) {
    // Do tilt every 8th tick of this function
    // TODO: Find a better way to handle this
    // Read accelerometer
    msa301_get_accel(&hi2c4, &acceleration_data_buffer[tilt_sample_offset * 3]);

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

    blit::tilt = Vec3(
      -(tilt_x / ACCEL_OVER_SAMPLE),
      -(tilt_y / ACCEL_OVER_SAMPLE),
      -(tilt_z / ACCEL_OVER_SAMPLE)
      );
    blit::tilt.normalize();

    last_tilt_update = blit::now();
  }

  if(blit::buttons & blit::MENU && !(blit_last_buttons & blit::MENU)) {
    blit_menu();
  }

  blit_last_buttons = blit::buttons;
  //flip_cycle_count = DWT->CYCCNT - scc;
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

void blit_switch_execution(void)
{
  // Stop the ADC DMA
  HAL_ADC_Stop_DMA(&hadc1);
  HAL_ADC_Stop_DMA(&hadc3);

  // Stop the audio
  HAL_TIM_Base_Stop_IT(&htim6);
  HAL_DAC_Stop(&hdac1, DAC_CHANNEL_2);

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

	volatile uint32_t uAddr = EXTERNAL_LOAD_ADDRESS;
	// enable qspi memory mapping if needed
	if(EXTERNAL_LOAD_ADDRESS >= 0x90000000)
		qspi_enable_memorymapped_mode();

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

void EnableUsTimer(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t GetUsTimer(void)
{
	uint32_t uTicksPerUs = SystemCoreClock / 1000000;
	return DWT->CYCCNT/uTicksPerUs;
}

uint32_t GetMaxUsTimer(void)
{
	uint32_t uTicksPerUs = SystemCoreClock / 1000000;
	return UINT32_MAX / uTicksPerUs;
}


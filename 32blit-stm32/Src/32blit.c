#include "string.h"

#include "32blit.h"
#include "main.h"
#include "adc.h"
#include "ltdc.h"
#include "tim.h"
#include "spi.h"
#include "spi-st7272a.h"
#include "i2c.h"
#include "i2c-msa301.h"
#include "i2c-bq24295.h"

#include "32blit.hpp"

using namespace blit;


extern void init();
extern void update(uint32_t time);
extern void render(uint32_t time);

extern char __ltdc_start;
extern char itcm_text_start;
extern char itcm_text_end;
extern char itcm_data;

blit::screen_mode mode = blit::screen_mode::lores;

/* configure the screen surface to point at the reserved LTDC framebuffer */
surface __ltdc((uint8_t *)&__ltdc_start, pixel_format::RGB565, size(320, 240));
uint8_t ltdc_buffer_id = 0;

surface __fb(((uint8_t *)&__ltdc_start) + (320 * 240 * 2), pixel_format::RGB, size(160, 120));

void blit_tick() {
    blit::tick(blit::now());
}

void blit_init() {
    ST7272A_RESET();

    st7272a_set_bgr();

    msa301_init(&hi2c4, MSA301_CONTROL2_POWR_MODE_NORMAL, 0x00, MSA301_CONTROL1_ODR_125HZ);
    //bq24295_init(&hi2c4);
    blit::backlight = 1.0f;
    blit::now = HAL_GetTick;
    blit::set_screen_mode = ::set_screen_mode;
    ::set_screen_mode(blit::lores);

    blit::update = ::update;
    blit::render = ::render;
    blit::init   = ::init;

    blit::init();
}

void blit_swap() {
    if (mode == blit::screen_mode::hires) {
        // LTDC framebuffer swap mode
        // 2 x 320x240 16-bit framebuffers are used alternately. Once drawing is
        // complete the data cache is invalidated and the LTDC hardware is pointed
        // to the freshly drawn framebuffer. Then the drawing framebuffer is swapped
        // for the next frame.

        // flip to non visible buffer for render
        ltdc_buffer_id = ltdc_buffer_id == 0 ? 1 : 0;
        blit::fb.data = (uint8_t *)(&__ltdc_start) + (ltdc_buffer_id * 320 * 240 * 2);
    }else {
        ltdc_buffer_id = 0;

        // set the LTDC layer framebuffer pointer shadow register
        LTDC_Layer1->CFBAR = (uint8_t *)(&__ltdc_start);
        // force LTDC driver to reload shadow registers
        LTDC->SRCR = LTDC_SRCR_IMR;
    }
}

void blit_flip() {
    if(mode == screen_mode::hires) {
        // HIRES mode
        SCB_CleanInvalidateDCache_by_Addr((uint32_t *)blit::fb.data, 320 * 240 * 2);

        // wait until next VSYNC period
        while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS));

        // set the LTDC layer framebuffer pointer shadow register
        LTDC_Layer1->CFBAR = (uint8_t *)(&__ltdc_start) + (ltdc_buffer_id * 320 * 240 * 2);
        // force LTDC driver to reload shadow registers
        LTDC->SRCR = LTDC_SRCR_IMR;
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

void blit_process_input() {
  // read x axis of joystick
  bool joystick_button = false;

  HAL_ADC_Start(&hadc1);
  ADC_update_joystick_axis(&hadc1, &blit::joystick.x);
  ADC_update_joystick_axis(&hadc1, &blit::joystick.y);
  HAL_ADC_Stop(&hadc1);

  HAL_ADC_Start(&hadc3);
  ADC_update_joystick_axis(&hadc3, &blit::hack_left);
  ADC_update_joystick_axis(&hadc3, &blit::hack_right);
  if (HAL_ADC_PollForConversion(&hadc3, 1000000) == HAL_OK)
  {
    blit::battery = 6.6 * HAL_ADC_GetValue(&hadc3) / 65535.0f;
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

  int16_t acceleration_data_buffer[3];
  acceleration_data_buffer[0] = 0;
  acceleration_data_buffer[1] = 0;
  acceleration_data_buffer[2] = 0;
  msa301_get_accel(&hi2c4, acceleration_data_buffer);

  blit::tilt = vec3(
    -acceleration_data_buffer[0],
    -acceleration_data_buffer[1],
    -acceleration_data_buffer[2]
    );
  blit::tilt.normalize();
}
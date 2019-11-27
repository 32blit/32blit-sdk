
#define BLIT_ENABLE_SD
#include "main.h"
#include "string.h"
#include "math.h"
#include "helpers.h"
#include "msa301.h"
#include "bq24295.h"
#include "32blit.hpp"
#include "graphics/color.hpp"
#ifdef BLIT_ENABLE_SD
#include "fatfs.h"
#endif
#include "usb_device.h"


#define I2C_READ_ACCEL
#define I2C_READ_PWR

extern void init();
extern void update(uint32_t time);
extern void render(uint32_t time);

//using namespace engine;
//using namespace graphics;
//using namespace utility;
using namespace blit;

/* peripheral handles */
/*ADC_HandleTypeDef hadc3;
DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch2;
TIM_HandleTypeDef htim6;

QSPI_HandleTypeDef hqspi;
SD_HandleTypeDef hsd1;
SPI_HandleTypeDef hspi1;
PCD_HandleTypeDef hpcd_USB_OTG_HS;*/
SPI_HandleTypeDef hspi1;
I2C_HandleTypeDef hi2c4;
ADC_HandleTypeDef hadc1;
LTDC_HandleTypeDef hltdc;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim3;


#ifdef BLIT_ENABLE_SD
// FF STUFF
FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;
char buffer[100];
// END FF STUFF
#endif

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

extern char itcm_text_start;
extern char itcm_text_end;
extern char itcm_data;

/* configure the screen surface to point at the reserved LTDC framebuffer */
extern char __ltdc_start;
surface __ltdc((uint8_t *)&__ltdc_start, pixel_format::RGB565, size(320, 240));
uint8_t ltdc_buffer_id = 0;

surface __fb(((uint8_t *)&__ltdc_start) + (320 * 240 * 2), pixel_format::RGB, size(160, 120));



blit::screen_mode mode = blit::screen_mode::lores;
void set_screen_mode(blit::screen_mode new_mode) {
  mode = new_mode;

  if(mode == blit::screen_mode::hires) {
    blit::fb = __ltdc;
  } else {
    blit::fb = __fb;
  }
}

static void MX_I2C4_Init(void);
static void MX_SPI1_Init(void);
static void MX_DMA_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM6_Init(void);
//static void MX_USB_OTG_HS_PCD_Init(void);
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);
void MX_LTDC_Init(void);
void MX_ADC1_Init(void);
void LCD_Init();

void process_input();

int16_t acceleration_data_buffer[3];

int main(void)
{
  //SystemInit();
  //SCB->CPACR |= ((3UL << 10*2) |             /* set CP10 Full Access               */
    //             (3UL << 11*2)  );           /* set CP11 Full Access               */

  // reset all peripherals, initializes flash interface and systick
  HAL_Init();

  // configure the clocks
  SystemClock_Config();

  // enable instruction and data caches, this results in a *huge*
  // performance boost (around 5x speedup)
  SCB_EnableICache();
  SCB_EnableDCache();

  // intialise peripherals
//  MX_DMA_Init();
//  MX_DAC1_Init();
//  MX_TIM6_Init();
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_TIM4_Init(); // VIBE_EN
  MX_LTDC_Init();
  LCD_Init();
  MX_ADC1_Init();
  MX_I2C4_Init();
  MX_SPI1_Init();

#ifdef BLIT_ENABLE_SD
	MX_FATFS_Init();
#endif
  
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1); // VIBE_EN
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);

  

  BACKLIGHT_ON();
  //USB_MODE_COMMS();
  //MX_USB_DEVICE_Init();

  //HAL_Delay(500);

#ifdef BLIT_ENABLE_SD
  bool SD_Error = 0;
  bool SD_Detected = 0;
  
  char SD_Label[12];
  
  BYTE work[FF_MAX_SS];

 /* if(f_mkfs("", FM_FAT32, 0, work, sizeof work) != FR_OK){
   SD_Error = 1;
 }*/


  uint8_t write_buffer[1024] = {0};
  for(auto x = 0; x < 1024; x++){
    write_buffer[x] = 'a';
  }
#endif



  // copy data blocks from flash into memory allocated by the linker
  // unsigned int itcm_text_size = (unsigned int)(&itcm_text_end - &itcm_text_start); // Pointer math should will work if these are declared as a char/byte type
  // memcpy(&itcm_text_start, &itcm_data, itcm_text_size);

  // setup the 32blit engine
  blit::now = HAL_GetTick;
  blit::set_screen_mode = ::set_screen_mode;
  ::set_screen_mode(blit::lores);

  blit::update = ::update;
  blit::render = ::render;
  blit::init   = ::init;

  //HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (uint32_t*)val, sizeof(val), DAC_ALIGN_12B_R);
  msa301_init(&hi2c4, MSA301_CONTROL2_POWR_MODE_NORMAL, 0x00, MSA301_CONTROL1_ODR_125HZ);

  int32_t adc_x, adc_y;

  blit::init();

  bool bq24295_ok = bq24295_init(&hi2c4);

#ifdef BLIT_ENABLE_SD
  if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11) == 1){
      if(f_mount(&fs, "", 1) != FR_OK){
      SD_Error = 1;
    }
    if(f_getlabel("", SD_Label, 0) != FR_OK){
      SD_Error = 1;
    }

    // Check freeSpace space
    if(f_getfree("", &fre_clust, &pfs) != FR_OK){
      SD_Error = 1;
    }

    totalSpace = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
    freeSpace = (uint32_t)(fre_clust * pfs->csize * 0.5);

    if(f_open(&fil, "test-pattern.bin", FA_READ) != FR_OK) {
      SD_Error = 1;
    }

    /*if(f_open(&fil, "test.txt", FA_OPEN_APPEND | FA_READ | FA_WRITE) != FR_OK){
        SD_Error = 1;
      }

    uint32_t t_start = blit::now();
    for(auto x = 0; x < 1024; x++){
      f_puts(write_buffer, &fil);
    }
    t_elapsed = blit::now() - t_start;

    // Close file
    if(f_close(&fil) != FR_OK) {
          SD_Error = 1;
    }*/

    // Unmount SDCARD
    /*if(f_mount(NULL, "", 1) != FR_OK) {
          SD_Error = 1;
    } */
  }
#endif

  while (true)
  {
    //blit::rgba c = blit::hsv_to_rgba((blit::now() % 5000) / 5000.0f, 1.0, 1.0);
    process_input();

    //__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, 2000 * (int(blit::now() / 100.0f) & 1));

    float scale_vibe = (sin(blit::now() / 1000.0f) + 1.0) / 2.0;
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, 2000 * scale_vibe);


    // RED Led
    float scale_a = (sin(blit::now() / 1000.0f) + 1.0) / 2.0;
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, 10000 * scale_a);

    // GREEN Led
    float scale_b = (cos(blit::now() / 1000.0f) + 1.0) / 2.0;
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_4, 5000 * scale_b);
  
    // BLUE Led
    float scale_c = (sin(blit::now() / 1000.0f) + 1.0) / 2.0;
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 10000 * scale_a);


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

    // render the current frame
    blit::tick(blit::now());

#ifdef BLIT_ENABLE_SD
    if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11) == 1){
      fb.pen(rgba(0, 255, 0));
      fb.text("SD DETECTED", &minimal_font[0][0], point(20, 32));
      fb.pen(rgba(255, 255, 255));
      fb.text(SD_Label, &minimal_font[0][0], point(20, 26));

      char buf_totalspace[60] = "";
      char buf_freespace[60] = "";
      sprintf(buf_totalspace, "Total: %d", (int)(totalSpace / 1024));
      sprintf(buf_freespace, "Free: %d", (int)(freeSpace / 1024));

      fb.text(buf_totalspace, &minimal_font[0][0], point(80, 32));
      fb.text(buf_freespace, &minimal_font[0][0], point(80, 26));

      if(SD_Detected == 0){
        SD_Detected = 1;
      }
    }

    uint32_t t_start = blit::now();
    for(auto y = 0; y < 240; y++) {
      uint8_t buf[320*3];
      uint32_t read;
      FRESULT err = f_read(&fil, buf, 320*3, &read);
      if(err == FR_DISK_ERR){
        fb.pen(rgba(255, 0, 0));
        fb.pixel(point(0,0));
      }
      if(err == FR_INT_ERR){
        fb.pen(rgba(255, 0, 0));
        fb.pixel(point(10,0));
      }
      if(err == FR_DENIED){
        fb.pen(rgba(255, 0, 0));
        fb.pixel(point(20,0));
      }
      if(err == FR_INVALID_OBJECT){
        fb.pen(rgba(255, 0, 255));
        fb.pixel(point(30,0));
      }
      if(err == FR_TIMEOUT){
        fb.pen(rgba(255, 0, 0));
        fb.pixel(point(40,0));
      }

      if(err == FR_OK){
        for(auto x = 0; x < 320; x++){
          int offset = x * 3;
          fb.pen(rgba(
            buf[offset + 2],
            buf[offset + 1],
            buf[offset]
            ));
          fb.pixel(point(x, y));
        }
      }
    }
    uint32_t t_elapsed = blit::now() - t_start;
    char buf_writetime[60] = "";
    sprintf(buf_writetime, "Time: %d Now: %d", (int)t_elapsed, (int)blit::now());
    fb.text(buf_writetime, &minimal_font[0][0], point(10, 10));

    f_lseek(&fil, 0);

    /* if(SD_Error) {
      fb.pen(rgba(255, 0, 0));
      fb.text("SD ERROR", &minimal_font[0][0], point(40, 25));
    }else{
      fb.pen(rgba(0, 255, 0));
      fb.text("SD OK", &minimal_font[0][0], point(40, 25));
    }*/
#endif

#ifdef I2C_READ_PWR
    if(bq24295_ok) {
      uint8_t bq24295_status = bq24295_get_status(&hi2c4);
      fb.pen(rgba(255, 255, 255));
      fb.text("BQ24295 OK: ", &minimal_font[0][0], point(80, 100));

      if(bq24295_status & 0b00001000){
      fb.text("PWR: GOOD ", &minimal_font[0][0], point(80, 110));
      }
      else {
      fb.text("PWR: NOT GOOD ", &minimal_font[0][0], point(80, 110));
      }
    }
    else
    {
      fb.pen(rgba(255, 255, 255));
      fb.text("BQ24295 FARKED", &minimal_font[0][0], point(80, 100));
    }
#endif

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, bq24295_ok ? 1 : 0);

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
/*
      blit::fb.pen(rgba(0, 0, 0));
      blit::fb.rectangle(rect(0, 0, 100, 30));
      blit::fb.pen(rgba(255, 255, 255));
      blit::fb.text(std::to_string(acceleration_data_buffer[0]), &minimal_font[0][0], point(0, 0));
      blit::fb.text(std::to_string(acceleration_data_buffer[1]), &minimal_font[0][0], point(0, 10));
      blit::fb.text(std::to_string(acceleration_data_buffer[2]), &minimal_font[0][0], point(0, 20));
*/
      uint16_t *dest = (uint16_t *)(&__ltdc_start);
      for(uint8_t y = 0; y < 120; y++) {
        // pixel double the current row while converting from RGBA to RGB565
        for(uint8_t x = 0; x < 160; x++) {
          uint8_t r = src->r >> 3;
          uint8_t g = src->g >> 2;
          uint8_t b = src->b >> 3;
          uint16_t c = (b << 11) | (g << 5) | (r);
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
}

//uint8_t vibe = 1;
//uint8_t vibe_trigger = 10;

void process_input() {

  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2));
  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_4));
  
  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0));

  // Tie USB switch to button A
  //HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2));

  // Tie backlight on/off to button X
  // HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0));


  /*vibe++;
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, vibe > vibe_trigger ? 1 : 0);
  if(vibe > vibe_trigger) {
      vibe = 0;
  }*/

  // read x axis of joystick
  uint16_t adc;
  bool joystick_button = false;

  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 1000000) == HAL_OK)
  {
    adc = HAL_ADC_GetValue(&hadc1);
    float f = adc / 4096.0f;
    f += -0.5f;
    blit::joystick.x = f;
  }
  /*if (HAL_ADC_PollForConversion(&hadc1, 1000000) == HAL_OK)
  {
    adc = HAL_ADC_GetValue(&hadc1);
    float f = adc / 4096.0f;
    joystick_button = f > 0.5;
  }*/
  if (HAL_ADC_PollForConversion(&hadc1, 1000000) == HAL_OK)
  {
    adc = HAL_ADC_GetValue(&hadc1);
    float f = adc / 4096.0f;
    f += -0.5f;
    blit::joystick.y = f;
  }
  HAL_ADC_Stop(&hadc1);

  // Read buttons

  blit::buttons =
    (!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_9)   ? blit::DPAD_UP    : 0) |
    (!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_14)  ? blit::DPAD_DOWN  : 0) |
    (!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10)  ? blit::DPAD_LEFT  : 0) |
    (!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_15)  ? blit::DPAD_RIGHT : 0) |
    (!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2)   ? blit::A          : 0) |
    (!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_4)   ? blit::B          : 0) |
    (!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0)   ? blit::X          : 0) |
    (!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_1)   ? blit::Y          : 0) |
    (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7)    ? blit::HOME       : 0) |  // INVERTED LOGIC!
    (!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_5)   ? blit::MENU       : 0) |
    (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5)   ? blit::JOYSTICK   : 0);

  // Tie shipping mode enable to HOME + MENU
  if((blit::buttons & blit::HOME) && (blit::buttons & blit::MENU)) {
    bq24295_enable_shipping_mode(&hi2c4);
  }

  // read accelerometer

  #ifdef I2C_READ_ACCEL
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
  #endif


/*
  sConfig.Channel = ADC_CHANNEL_8;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK)
  {
    adc = HAL_ADC_GetValue(&hadc1);
    float f = adc / 4096.0f;
    f -= -0.5f;
    blit::joystick.y = f;
  }
  HAL_ADC_Stop(&hadc1);*/
}

void LCD_Init() {
  LCD_RESET()

/*  LCD_CS(true);

  uint8_t display_on[2] = {0x01, 0b00001001};
  //HAL_SPI_Transmit(&hspi1, display_on, 2, HAL_MAX_DELAY);


  LCD_CS(false);*/
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_SPI1
                              |RCC_PERIPHCLK_SDMMC|RCC_PERIPHCLK_ADC
                              |RCC_PERIPHCLK_I2C4|RCC_PERIPHCLK_USB
                              |RCC_PERIPHCLK_QSPI;
  PeriphClkInitStruct.PLL2.PLL2M = 4;
  PeriphClkInitStruct.PLL2.PLL2N = 12; //9;
  PeriphClkInitStruct.PLL2.PLL2P = 1;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 4096; //3072;
  PeriphClkInitStruct.PLL3.PLL3M = 32;
  PeriphClkInitStruct.PLL3.PLL3N = 129;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 2;
  PeriphClkInitStruct.PLL3.PLL3R = 28; // 46 == 30Hz - 28 == 50Hz
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_1;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_D1HCLK;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_D3PCLK1;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable USB Voltage detector
  */
  HAL_PWREx_EnableUSBVoltageDetector();
}

// void SystemClock_Config(void)
// {

//   RCC_OscInitTypeDef RCC_OscInitStruct;
//   RCC_ClkInitTypeDef RCC_ClkInitStruct;
//   RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

//     /**Supply configuration update enable
//     */
//   MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

//     /**Configure the main internal regulator output voltage
//     */
//   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

//   while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY)
//   {

//   }
//     /**Macro to configure the PLL clock source
//     */
//   __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);

//     /**Initializes the CPU, AHB and APB busses clocks
//     */
//   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI;
//   RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
//   RCC_OscInitStruct.HSICalibrationValue = 16;
//   RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
//   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
//   RCC_OscInitStruct.PLL.PLLM = 32;
//   RCC_OscInitStruct.PLL.PLLN = 400;
//   RCC_OscInitStruct.PLL.PLLP = 2;
//   RCC_OscInitStruct.PLL.PLLQ = 2;
//   RCC_OscInitStruct.PLL.PLLR = 2;
//   RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
//   RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
//   RCC_OscInitStruct.PLL.PLLFRACN = 0;
//   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//   {
//     _Error_Handler(__FILE__, __LINE__);
//   }

//     /**Initializes the CPU, AHB and APB busses clocks
//     */
//   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
//                               |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
//                               |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
//   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
//   RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
//   RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV4;
//   RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
//   RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
//   RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
//   RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

//   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
//   {
//     _Error_Handler(__FILE__, __LINE__);
//   }

//   PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_SPI1
//                               |RCC_PERIPHCLK_SDMMC|RCC_PERIPHCLK_ADC
//                               |RCC_PERIPHCLK_I2C4|RCC_PERIPHCLK_USB
//                               |RCC_PERIPHCLK_QSPI;
//   PeriphClkInitStruct.PLL2.PLL2M = 32;
//   PeriphClkInitStruct.PLL2.PLL2N = 129;
//   PeriphClkInitStruct.PLL2.PLL2P = 2;
//   PeriphClkInitStruct.PLL2.PLL2Q = 2;
//   PeriphClkInitStruct.PLL2.PLL2R = 2;
//   PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_1;
//   PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
//   PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
//   PeriphClkInitStruct.PLL3.PLL3M = 32;
//   PeriphClkInitStruct.PLL3.PLL3N = 129;
//   PeriphClkInitStruct.PLL3.PLL3P = 2;
//   PeriphClkInitStruct.PLL3.PLL3Q = 2;
//   PeriphClkInitStruct.PLL3.PLL3R = 28;
//   PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_1;
//   PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
//   PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
//   PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_D1HCLK;
//   PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
//   PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
//   PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
//   PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_D3PCLK1;
//   PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
//   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
//   {
//     _Error_Handler(__FILE__, __LINE__);
//   }

//     /**Configure the Systick interrupt time
//     */
//   HAL_SYSTICK_Config(SystemCoreClock/1000);

//     /**Configure the Systick
//     */
//   HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

//   /* SysTick_IRQn interrupt configuration */
//   HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
// }

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_MultiModeTypeDef multimode;
  ADC_ChannelConfTypeDef sConfig;

    /**Common config
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  //hadc1.Init.BoostMode = ENABLE;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the ADC multi-mode
    */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfig.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;

  sConfig.Channel = ADC_CHANNEL_4; // Joystick X
  sConfig.Rank = 1;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  //sConfig.Channel = ADC_CHANNEL_8; // Joystick Button
  //sConfig.Rank = 2;
  //HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  sConfig.Channel = ADC_CHANNEL_10; // Joystick Y
  sConfig.Rank = 2;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);




}

/* LTDC init function */
static void MX_LTDC_Init(void)
{

  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 3;
  hltdc.Init.VerticalSync = 3;
  hltdc.Init.AccumulatedHBP = 46;
  hltdc.Init.AccumulatedVBP = 15;
  hltdc.Init.AccumulatedActiveW = 366;
  hltdc.Init.AccumulatedActiveH = 495;
  hltdc.Init.TotalWidth = 374;
  hltdc.Init.TotalHeigh = 497;
  hltdc.Init.Backcolor.Blue = 30;
  hltdc.Init.Backcolor.Green = 40;
  hltdc.Init.Backcolor.Red = 50;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    LED_TWO_FLASH()
    _Error_Handler(__FILE__, __LINE__);
  }


/*
 LED_TWO_FLASH()

  // Configure the HS, VS, DE and PC polarity
  LTDC->GCR &= ~(LTDC_GCR_HSPOL | LTDC_GCR_VSPOL | LTDC_GCR_DEPOL | LTDC_GCR_PCPOL);
  LTDC->GCR |=  (uint32_t)(LTDC_HSPOLARITY_AL | LTDC_VSPOLARITY_AL | LTDC_DEPOLARITY_AL | LTDC_PCPOLARITY_IPC);

  // Set Synchronization size
  LTDC->SSCR &= ~(LTDC_SSCR_VSH | LTDC_SSCR_HSW);
  LTDC->SSCR |= ((3 << 16) | 3);   // horizontal and vertical sync

  // Set Accumulated Back porch
  LTDC->BPCR &= ~(LTDC_BPCR_AVBP | LTDC_BPCR_AHBP);
  LTDC->BPCR |= ((46 << 16) | 15);

  // Set Accumulated Active Width
  LTDC->AWCR &= ~(LTDC_AWCR_AAH | LTDC_AWCR_AAW);
  LTDC->AWCR |= ((366 << 16) | 495);

  // Set total width and height
  LTDC->TWCR &= ~(LTDC_TWCR_TOTALH | LTDC_TWCR_TOTALW);
  LTDC->TWCR |= ((374 << 16) | 497);

  // Set the background color value
  LTDC->BCCR &= ~(LTDC_BCCR_BCBLUE | LTDC_BCCR_BCGREEN | LTDC_BCCR_BCRED);
  LTDC->BCCR |= 0x00FF00FF; // magenta default background colour

  // Enable the Transfer Error and FIFO underrun interrupts
  LTDC->IER |= LTDC_IT_TE | LTDC_IT_FU;

  // Enable LTDC by setting LTDCEN bit
  LTDC->GCR |= LTDC_GCR_LTDCEN;

 LED_TWO_FLASH()
*/


LTDC_LayerCfgTypeDef pLayerCfg;

  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 320;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 240;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 255;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = (uint32_t)&__ltdc_start;
  pLayerCfg.ImageWidth = 320;
  pLayerCfg.ImageHeight = 240;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, LTDC_LAYER_1) != HAL_OK)
  {
    LED_TWO_FLASH()
    _Error_Handler(__FILE__, __LINE__);
  }

/*
  // configure the LTDC layers
  LTDC_Layer1->WHPCR &= ~(LTDC_LxWHPCR_WHSTPOS | LTDC_LxWHPCR_WHSPPOS);
  LTDC_Layer1->WHPCR = ((0 + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> 16) + 1) | ((320 + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> 16)) << 16));

  // Configure the vertical start and stop position
  LTDC_Layer1->WVPCR &= ~(LTDC_LxWVPCR_WVSTPOS | LTDC_LxWVPCR_WVSPPOS);
  LTDC_Layer1->WVPCR  = ((0 + (LTDC->BPCR & LTDC_BPCR_AVBP) + 1) | ((240 + (LTDC->BPCR & LTDC_BPCR_AVBP)) << 16));

  // Specifies the pixel format
  LTDC_Layer1->PFCR &= ~(LTDC_LxPFCR_PF);
  LTDC_Layer1->PFCR = LTDC_PIXEL_FORMAT_RGB565;

  // Configure the default color values
  LTDC_Layer1->DCCR &= ~(LTDC_LxDCCR_DCBLUE | LTDC_LxDCCR_DCGREEN | LTDC_LxDCCR_DCRED | LTDC_LxDCCR_DCALPHA);
  LTDC_Layer1->DCCR = 0xFF001111; // cyan default background colour

  // Specifies the constant alpha value
  LTDC_Layer1->CACR &= ~(LTDC_LxCACR_CONSTA);
  LTDC_Layer1->CACR = 255;

  // Specifies the blending factors
  LTDC_Layer1->BFCR &= ~(LTDC_LxBFCR_BF2 | LTDC_LxBFCR_BF1);
  LTDC_Layer1->BFCR = (LTDC_BLENDING_FACTOR1_CA | LTDC_BLENDING_FACTOR2_CA);

  // Configure the color frame buffer start address
  LTDC_Layer1->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
  LTDC_Layer1->CFBAR = (uint32_t)frame_buffer;

  // Configure the color frame buffer pitch in byte
  LTDC_Layer1->CFBLR  &= ~(LTDC_LxCFBLR_CFBLL | LTDC_LxCFBLR_CFBP);
  LTDC_Layer1->CFBLR  = (((320 * 2) << 16) | (((320) * 2)  + 7));

  // Configure the frame buffer line number
  LTDC_Layer1->CFBLNR  &= ~(LTDC_LxCFBLNR_CFBLNBR);
  LTDC_Layer1->CFBLNR  = 240;

  // Enable LTDC_Layer by setting LEN bit
  LTDC_Layer1->CR |= LTDC_LxCR_LEN;


  // Set the Immediate Reload type
  LTDC->SRCR = LTDC_SRCR_IMR;
*/

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


/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 100;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 5000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) // LED_BLUE
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) // LED_RED
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK) // LED_GREEN
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 117;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 10000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM4 GPIO Configuration    
    PB6     ------> TIM4_CH1 
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  // why is this needed?
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  // action buttons
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  // dpad switches
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_10 | GPIO_PIN_9, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_10 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  // home / menu buttons
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5 | GPIO_PIN_7, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  // joystick button
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // led indicator pins RED/GREEN
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // led indicator pins BLUE
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2 | GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_15, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);*/

  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);


/*
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);*/

  // backlight enable pin
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  // dac output pin
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // analog joystick pins
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); 

  // amp shutdown
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // SD Chip Select
  /*HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);*/

  // SD Card Detect
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);


    // PORTB Pin 6, Timer 4, Channel 1  VIBE_EN
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
  GPIO_InitTypeDef GPIO_Timer4InitStruct = {0};
  GPIO_Timer4InitStruct.Pin = GPIO_PIN_6;
  GPIO_Timer4InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_Timer4InitStruct.Pull = GPIO_NOPULL;
  GPIO_Timer4InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_Timer4InitStruct.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOB, &GPIO_Timer4InitStruct);
}


/* I2C4 init function */
static void MX_I2C4_Init(void)
{

  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x10C0ECFF; // ORIGINAL VALUE, WTF IS THIS SO CRYPTIC!!?!?!?
  //hi2c4.Init.Timing = 0x103039FF; // WHO KNOWS!
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Analogue filter
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Digital filter
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/* DAC1 init function */
/*
static void MX_DAC1_Init(void)
{

  DAC_ChannelConfTypeDef sConfig;

    //**DAC Initialization

  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    //**DAC channel OUT2 config

  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = 0x00000004; // timer 6
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}*/

/* TIM6 init function */
/*static void MX_TIM6_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 400;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 1000;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}*/

/**
  * Enable DMA controller clock
  */
/*static void MX_DMA_Init(void)
{
  // DMA controller clock enable
  __HAL_RCC_DMA1_CLK_ENABLE();

  // DMA interrupt init
  // DMA1_Stream0_IRQn interrupt configuration
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

}*/

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}




#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

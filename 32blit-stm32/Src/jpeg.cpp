#include <cstdint>

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_jpeg.h"

extern "C" {
#include "JPEG/jpeg_utils.h"
}

#include "engine/file.hpp"
#include "graphics/jpeg.hpp"

static bool jpeg_tables_initialised = false;

static JPEG_HandleTypeDef jpeg_handle;
static const int jpeg_max_block_len = 768; // can fit a whole number of all block sizes
static uint8_t jpeg_dec_block[jpeg_max_block_len];
static uint8_t *jpeg_in_buf = nullptr, *jpeg_out_buf = nullptr;
static uint32_t jpeg_in_len = 0, jpeg_in_off = 0;
static blit::File *jpeg_in_file = nullptr;
static uint32_t jpeg_out_block = 0;
static JPEG_YCbCrToRGB_Convert_Function jpeg_conv_func = nullptr;

void HAL_JPEG_InfoReadyCallback(JPEG_HandleTypeDef *hjpeg, JPEG_ConfTypeDef *pInfo) {
  // allocate output RGB buffer
  if(pInfo->ImageWidth * pInfo->ImageHeight != 0) {
    uint32_t jpeg_out_len = pInfo->ImageWidth * pInfo->ImageHeight * 3;
    jpeg_out_buf = new uint8_t[jpeg_out_len];
  }
  // else fail horribly

  // get conversion func
  uint32_t num_mcus;
  JPEG_GetDecodeColorConvertFunc(pInfo, &jpeg_conv_func, &num_mcus);
}

void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData) {
  jpeg_in_off += NbDecodedData;

  if(jpeg_in_file) {
    jpeg_in_len = jpeg_in_file->read(jpeg_in_off, 1024, (char *)jpeg_in_buf);
    HAL_JPEG_ConfigInputBuffer(&jpeg_handle, jpeg_in_buf, jpeg_in_len);
  } else if(jpeg_in_off < jpeg_in_len)
    HAL_JPEG_ConfigInputBuffer(&jpeg_handle, jpeg_in_buf + jpeg_in_off, jpeg_in_len - jpeg_in_off);
}

void HAL_JPEG_DataReadyCallback(JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength) {
  uint32_t conv_count;
  jpeg_out_block += jpeg_conv_func(jpeg_dec_block, jpeg_out_buf, jpeg_out_block, OutDataLength, &conv_count);
}


blit::JPEGImage blit_decode_jpeg_buffer(uint8_t *ptr, uint32_t len) {
  if(!jpeg_tables_initialised) {
    JPEG_InitColorTables();
    jpeg_tables_initialised = true;
  }

  jpeg_in_buf = ptr;
  jpeg_in_len = len;
  jpeg_in_off = 0;
  jpeg_in_file = nullptr;
  
  jpeg_out_block = 0;

  jpeg_handle.Instance = JPEG;
  HAL_JPEG_Init(&jpeg_handle);

  auto status = HAL_JPEG_Decode(&jpeg_handle, ptr, len, jpeg_dec_block, jpeg_max_block_len, 0xFFFFFFFF);

  JPEG_ConfTypeDef conf;
  HAL_JPEG_GetInfo(&jpeg_handle, &conf);

  HAL_JPEG_DeInit(&jpeg_handle);

  return {blit::Size(conf.ImageWidth, conf.ImageHeight), jpeg_out_buf};
}

blit::JPEGImage blit_decode_jpeg_file(std::string filename) {
  blit::File file(filename);

  if(!file.is_open())
    return {};

  jpeg_in_file = &file;

  if(!jpeg_tables_initialised) {
    JPEG_InitColorTables();
    jpeg_tables_initialised = true;
  }

  jpeg_in_buf = new uint8_t[1024];
  jpeg_in_len = file.read(0, 1024, (char *)jpeg_in_buf);
  jpeg_in_off = 0;
  
  jpeg_out_block = 0;

  jpeg_handle.Instance = JPEG;
  HAL_JPEG_Init(&jpeg_handle);

  auto status = HAL_JPEG_Decode(&jpeg_handle, jpeg_in_buf, jpeg_in_len, jpeg_dec_block, jpeg_max_block_len, 0xFFFFFFFF);

  JPEG_ConfTypeDef conf;
  HAL_JPEG_GetInfo(&jpeg_handle, &conf);

  HAL_JPEG_DeInit(&jpeg_handle);

  delete[] jpeg_in_buf;

  return {blit::Size(conf.ImageWidth, conf.ImageHeight), jpeg_out_buf};
}

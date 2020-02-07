/*! \file utility.cpp
*/
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>

#include "utility.hpp"
#include "../types/pixel_format.hpp"

using namespace blit;

namespace utility {

  uint8_t utility_buffer[512];

  enum tga_status {
    OK = 0,
    FORMAT_NOT_SUPPORTED = -1,
    BIT_DEPTH_NOT_SUPPORTED = -2,
    FILE_READ_ERROR = -3,
  };

  tga tga_header(const char * file) {
    FILE *fp = fopen(file, "rb");

    tga tga;
    fread((void *)&tga, sizeof(tga), 1, fp);

    return tga;
  }
  
  int8_t tga_load(const char * file, void *data, bool auto_alpha) {
    FILE *fp = fopen(file, "rb");

    tga tga;
    fread((void *)&tga, sizeof(tga), 1, fp);

    // ensure uncompressed RGB pixel data
    if (tga.data_type != 2) {
      fclose(fp);
      return tga_status::FORMAT_NOT_SUPPORTED;
    }
      
    // ensure 32 bits per pixel
    if (tga.bpp != 32 && tga.bpp != 24) {
      fclose(fp);
      return tga_status::BIT_DEPTH_NOT_SUPPORTED;
    }    

    uint32_t bytes = 0;

    if (tga.bpp == 32) {
      bytes = tga.width * tga.height * 4;
      uint16_t row = tga.height - 1;

      while (bytes > 0 && !feof(fp)) {
        Pen *p = ((Pen *)data) + (row * tga.width);
        uint32_t bytes_read = fread((void *)utility_buffer, sizeof(utility_buffer[0]), tga.width * 4, fp);
        for (uint32_t i = 0; i < bytes_read; i += 4) {
          p->r = utility_buffer[i + 2];
          p->g = utility_buffer[i + 1];
          p->b = utility_buffer[i + 0];

          if (auto_alpha && (p->r + p->g + p->b) == 0) {
            p->a = 0;
          }else {
            p->a = utility_buffer[i + 3];
          }          

          p++;
        }

        bytes -= bytes_read;
        row--;
      }
    }

    if (tga.bpp == 24) {
      bytes = tga.width * tga.height * 3;
      uint16_t row = tga.height - 1;

      while (bytes > 0 && !feof(fp)) {
        Pen *p = ((Pen *)data) + (row * tga.width);
        uint32_t bytes_read = fread((void *)utility_buffer, sizeof(utility_buffer[0]), tga.width * 3, fp);
        for (uint32_t i = 0; i < bytes_read; i += 3) {
          p->r = utility_buffer[i + 2];
          p->g = utility_buffer[i + 1];
          p->b = utility_buffer[i + 0];

          if (auto_alpha && (p->r + p->g + p->b) == 0) {
            p->a = 0;
          } else {
            p->a = 255;
          }
          
          p++;
        }

        bytes -= bytes_read;
        row--;        
      }
    }

    // we didn't get as many bytes as we expected - that's bad!
    if (bytes != 0) {
      fclose(fp);
      return tga_status::FILE_READ_ERROR;
    }

    fclose(fp);
    
    return tga_status::OK;
  }

  int8_t map_load(const char * file, uint8_t *data) {
    FILE *fp = fopen(file, "rb");

    uint8_t width, height;
    fread((void *)&width, sizeof(width), 1, fp);
    fread((void *)&height, sizeof(height), 1, fp);

    uint32_t bytes = width * height;

    while (bytes > 0 && !feof(fp)) {      
      uint32_t bytes_read = fread((void *)data, sizeof(uint8_t), bytes, fp);
      data += bytes_read;
      bytes -= bytes_read;
    }

/*
    uint32_t bytes = 0;

    if (tga.bpp == 32) {
      bytes = tga.width * tga.height * 4;
      uint16_t row = tga.height - 1;

      while (bytes > 0 && !feof(fp)) {
        rgba *p = ((rgba *)data) + (row * tga.width);
        uint32_t bytes_read = fread((void *)utility_buffer, sizeof(utility_buffer[0]), tga.width * 4, fp);
        for (uint32_t i = 0; i < bytes_read; i += 4) {
          p->r = utility_buffer[i + 2];
          p->g = utility_buffer[i + 1];
          p->b = utility_buffer[i + 0];

          if (auto_alpha && (p->r + p->g + p->b) == 0) {
            p->a = 0;
          }
          else {
            p->a = utility_buffer[i + 3];
          }

          p++;
        }

        bytes -= bytes_read;
        row--;
      }
    }

    if (tga.bpp == 24) {
      bytes = tga.width * tga.height * 3;
      uint16_t row = tga.height - 1;

      while (bytes > 0 && !feof(fp)) {
        rgba *p = ((rgba *)data) + (row * tga.width);
        uint32_t bytes_read = fread((void *)utility_buffer, sizeof(utility_buffer[0]), tga.width * 3, fp);
        for (uint32_t i = 0; i < bytes_read; i += 3) {
          p->r = utility_buffer[i + 2];
          p->g = utility_buffer[i + 1];
          p->b = utility_buffer[i + 0];

          if (auto_alpha && (p->r + p->g + p->b) == 0) {
            p->a = 0;
          }
          else {
            p->a = 255;
          }

          p++;
        }

        bytes -= bytes_read;
        row--;
      }
    }

    // we didn't get as many bytes as we expected - that's bad!
    if (bytes != 0) {
      fclose(fp);
      return tga_status::FILE_READ_ERROR;
    }

    fclose(fp);

    return tga_status::OK;*/

    return 1;
  }

  tga     tga_header(const std::string & file) {
    return tga_header(file.c_str());
  }
  
  int8_t  tga_load(const std::string & file, void *data, bool auto_alpha) {
    return tga_load(file.c_str(), data, auto_alpha);
  }
  
  int8_t  map_load(const std::string & file, uint8_t *data) {
    return map_load(file.c_str(), data);
  }
}
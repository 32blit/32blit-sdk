#pragma once

#include <string>

namespace utility {

  #pragma pack(push, 1)
  struct tga {
    uint8_t   id_length;
    uint8_t   colour_map_type;
    uint8_t   data_type;
    uint16_t  colour_map_origin;
    uint16_t  colour_map_length;
    uint8_t   colour_map_depth;
    uint16_t  x_origin;
    uint16_t  y_origin;
    uint16_t  width;
    uint16_t  height;
    uint8_t   bpp;
    uint8_t   descriptor;
  };
  #pragma pack(pop)

  tga     tga_header(std::string file);
  int8_t  tga_load(std::string file, void *data, bool auto_alpha);
  int8_t  map_load(std::string file, uint8_t *data);
  
}
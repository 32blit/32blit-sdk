cmake_minimum_required(VERSION 3.9)
project (jpeg)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)

if(32BLIT_PICO)
    # No hardware JPEG decoding
    return()
endif()

blit_executable (jpeg jpeg.cpp)
blit_assets_yaml (jpeg assets.yml)
blit_metadata (jpeg metadata.yml)

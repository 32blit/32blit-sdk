cmake_minimum_required(VERSION 3.9)
project (audio-wave)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)

if(32BLIT_PICO)
    # PicoSystem does not support wave audio
    return()
endif()

blit_executable (audio-wave audio-wave.cpp)
blit_assets_yaml (audio-wave assets.yml)
blit_metadata (audio-wave metadata.yml)

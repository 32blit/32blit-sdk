cmake_minimum_required(VERSION 3.9)
project (waveform-demo)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)

if(32BLIT_PICO)
    # PicoSystem does not support wave audio
    return()
endif()

blit_executable (waveform-demo waveform-demo.cpp)
blit_metadata (waveform-demo metadata.yml)

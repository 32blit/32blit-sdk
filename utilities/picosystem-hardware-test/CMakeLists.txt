cmake_minimum_required(VERSION 3.9)
project (picosystem-hardware-test)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)

if(NOT PICO_BOARD STREQUAL "pimoroni_picosystem")
    # Hooks into the Pico SDK to get battery charge and VBUS status
    return()
endif()

blit_executable (picosystem-hardware-test hardware-test.cpp)
blit_metadata (picosystem-hardware-test metadata.yml)

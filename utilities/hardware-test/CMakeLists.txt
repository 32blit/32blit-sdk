cmake_minimum_required(VERSION 3.9)
project (hardware-test)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)

if(PICO_BOARD STREQUAL "pimoroni_picosystem")
    # PicoSystem has its own picosystem-hardware-test
    return()
endif()

blit_executable (hardware-test hardware-test.cpp)
blit_metadata (hardware-test metadata.yml)

cmake_minimum_required(VERSION 3.9)
project (picosystem-hardware-test)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)

if(NOT 32BLIT_PICO)
    # Hooks into the Pico SDK to get battery charge and VBUS status
    return()
endif()

blit_executable (picosystem-hardware-test hardware-test.cpp)
blit_metadata (picosystem-hardware-test metadata.yml)
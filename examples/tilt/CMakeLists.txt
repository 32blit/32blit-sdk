cmake_minimum_required(VERSION 3.9)
project (tilt)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)

if(32BLIT_PICO)
    # No accelerometer
    return()
endif()

blit_executable (tilt tilt.cpp)
blit_metadata (tilt metadata.yml)

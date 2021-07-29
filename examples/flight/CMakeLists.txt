cmake_minimum_required(VERSION 3.9)
project (flight)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)

if(32BLIT_PICO)
    # This example uses too much RAM (at run time)
    return()
endif()

blit_executable (flight flight.cpp)
blit_assets_yaml (flight assets.yml)
blit_metadata (flight metadata.yml)

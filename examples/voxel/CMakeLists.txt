cmake_minimum_required(VERSION 3.9)
project (voxel)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)

if(32BLIT_PICO)
    # This example uses too much RAM and flash
    return()
endif()

blit_executable (voxel voxel.cpp)
blit_assets_yaml(voxel assets.yml)
blit_metadata (voxel metadata.yml)

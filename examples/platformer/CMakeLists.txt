cmake_minimum_required(VERSION 3.9)
project (platformer)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)
blit_executable (platformer platformer.cpp)
blit_assets_yaml (platformer assets.yml)
blit_metadata (platformer metadata.yml)

# This removes support for set_screen_mode(hires) for Pico-based devices
# freeing up some extra RAM
target_compile_definitions(platformer PRIVATE ALLOW_HIRES=0)

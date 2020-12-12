cmake_minimum_required(VERSION 3.8)
project (launcher)
include (../32blit.cmake)

blit_executable(launcher firmware.cpp metadata.cpp)
blit_assets_yaml(launcher assets.yml)
blit_metadata(launcher metadata.yml)

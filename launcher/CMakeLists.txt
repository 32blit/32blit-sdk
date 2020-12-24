cmake_minimum_required(VERSION 3.8)
project (launcher)
include (../32blit.cmake)

blit_executable(launcher launcher.cpp)
target_link_libraries(launcher LauncherShared)
blit_assets_yaml(launcher assets.yml)
blit_metadata(launcher metadata.yml)

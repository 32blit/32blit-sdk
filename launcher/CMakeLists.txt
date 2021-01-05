cmake_minimum_required(VERSION 3.9)
project (launcher)
include (../32blit.cmake)

blit_executable(launcher launcher.cpp theme.cpp credits.cpp)
target_link_libraries(launcher LauncherShared)
blit_assets_yaml(launcher assets.yml)
blit_metadata(launcher metadata.yml)

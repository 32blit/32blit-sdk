cmake_minimum_required(VERSION 3.9...3.26)
project (launcher)
find_package (32BLIT CONFIG REQUIRED PATHS ..)

blit_executable(launcher launcher.cpp theme.cpp credits.cpp)
target_link_libraries(launcher LauncherShared)
blit_assets_yaml(launcher assets.yml)
blit_metadata(launcher metadata.yml)

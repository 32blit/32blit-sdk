
cmake_minimum_required(VERSION 3.8)
project (saves)
set(32BLIT_PATH "../../" CACHE PATH "Path to 32blit.cmake")
include (${32BLIT_PATH}/32blit.cmake)

blit_executable (saves saves.cpp)
blit_metadata (saves metadata.yml)
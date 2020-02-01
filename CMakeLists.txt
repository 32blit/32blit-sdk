cmake_minimum_required(VERSION 3.1)
project(32blit)
include(32blit.cmake)
add_subdirectory(examples)

if(${CMAKE_SYSTEM_NAME} STREQUAL Generic)
    add_subdirectory(firmware)
endif()
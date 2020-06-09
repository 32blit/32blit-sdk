cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0069 NEW)
project(32blit)
include(32blit.cmake)
add_subdirectory(examples)

if(${CMAKE_SYSTEM_NAME} STREQUAL Generic)
    add_subdirectory(32blit-stm32)
    add_subdirectory(firmware)
endif()

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL Generic AND NOT EMSCRIPTEN)
    add_subdirectory(tools/src)
endif()

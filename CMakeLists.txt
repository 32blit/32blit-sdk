cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0069 NEW)
project(32blit)

if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4244")
endif()

include(32blit.cmake)
add_subdirectory(examples)

if(${CMAKE_SYSTEM_NAME} STREQUAL Generic)
    add_subdirectory(32blit-stm32)
    add_subdirectory(firmware)
    add_subdirectory(fw-flash)
endif()

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL Generic AND NOT EMSCRIPTEN)
    add_subdirectory(tools/src)
endif()

# include dist files in install
install(DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}/dist/
    DESTINATION .
)

# setup release packages
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
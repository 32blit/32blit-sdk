cmake_minimum_required(VERSION 3.9)
project(32blit)

if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4244")
endif()

include(32blit.cmake)
add_subdirectory(examples)

add_subdirectory(launcher-shared)

if(${CMAKE_SYSTEM_NAME} STREQUAL Generic)
    add_subdirectory(32blit-stm32)
    add_subdirectory(firmware)
    add_subdirectory(firmware-update)
endif()

add_subdirectory(launcher)

# include dist files in install
install(DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}/dist/
    DESTINATION .
)

# setup release packages
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
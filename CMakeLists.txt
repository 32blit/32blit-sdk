cmake_minimum_required(VERSION 3.9...3.26)

# has to be before project
include(32blit-pico/sdk_import.cmake)

project(32blit)

if(MSVC)
    add_compile_options("/W4" "/wd4244" "/wd4324" "/wd4458" "/wd4100")
else()
    add_compile_options("-Wall" "-Wextra" "-Wdouble-promotion" "-Wno-unused-parameter")
endif()

find_package (32BLIT CONFIG REQUIRED PATHS .)
add_subdirectory(utilities)

add_subdirectory(launcher-shared)

if(32BLIT_HW)
    add_subdirectory(32blit-stm32)
    add_subdirectory(firmware)
    add_subdirectory(firmware-update)
endif()

if(NOT 32BLIT_PICO)
    add_subdirectory(launcher)
endif()

# if the examples are there, build them too
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/CMakeLists.txt)
    add_subdirectory(examples)
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

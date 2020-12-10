cmake_minimum_required(VERSION 3.8)
project (fw-flash)
include (../32blit.cmake)

# embed the built firmware
add_custom_command(
    COMMAND ${PYTHON_EXECUTABLE} -m ttblit raw --force --input_file $<TARGET_FILE_DIR:firmware>/$<TARGET_FILE_BASE_NAME:firmware>.bin --symbol_name firmware_data  --output_file firmware.hpp
    OUTPUT firmware.hpp
)

blit_executable(magic fw-flash.cpp firmware.hpp)

add_dependencies(magic firmware)
target_include_directories(magic PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
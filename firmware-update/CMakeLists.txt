cmake_minimum_required(VERSION 3.8)
project (firmware-update)
include (../32blit.cmake)

# embed the built firmware
add_custom_command(
    COMMAND ${PYTHON_EXECUTABLE} -m ttblit raw --force --input_file $<TARGET_FILE_DIR:firmware>/firmware.bin --symbol_name firmware_data  --output_file firmware.hpp
    OUTPUT firmware.hpp
)

blit_executable(firmware-update firmware-update.cpp firmware.hpp)

# generate metadata with version
if(DEFINED ENV{TRAVIS_TAG})
    set(FIRMWARE_VERSION $ENV{TRAVIS_TAG})
    set_target_properties(firmware-update PROPERTIES OUTPUT_NAME firmware-update-${FIRMWARE_VERSION})
else()
	set(FIRMWARE_VERSION "DEV")
endif()
configure_file(metadata.yml.in metadata.yml)

blit_metadata(firmware-update ${CMAKE_CURRENT_BINARY_DIR}/metadata.yml)

add_dependencies(firmware-update firmware)
target_include_directories(firmware-update PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
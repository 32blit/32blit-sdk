if(TARGET BlitHalPico)
    return()
endif()

# prevent find_package errors in pico_add_uf2_output later
set(PICO_SDK_VERSION_MAJOR ${PICO_SDK_VERSION_MAJOR} PARENT_SCOPE)
set(PICO_SDK_VERSION_MINOR ${PICO_SDK_VERSION_MINOR} PARENT_SCOPE)
set(PICO_SDK_VERSION_REVISION ${PICO_SDK_VERSION_REVISION} PARENT_SCOPE)

add_subdirectory(../32blit-shared ../32blit-shared)

add_library(BlitHalPico INTERFACE)
target_sources(BlitHalPico INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/blit_launch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/display.cpp
    ${CMAKE_CURRENT_LIST_DIR}/file.cpp
    ${CMAKE_CURRENT_LIST_DIR}/led.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/multiplayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/overlay.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
)

target_link_libraries(BlitHalPico INTERFACE
    hardware_dma hardware_i2c hardware_pio hardware_pwm hardware_spi
    pico_multicore pico_stdlib pico_unique_id pico_rand
    tinyusb_device
    FatFsBlitAPI
    LauncherShared
)

target_include_directories(BlitHalPico INTERFACE
    ${CMAKE_CURRENT_LIST_DIR} # for tusb_config
)

target_compile_definitions(BlitHalPico INTERFACE
    PICO_AUDIO_DMA_IRQ=1
)

target_compile_options(BlitHalPico INTERFACE
    -Wno-ignored-qualifiers # pico-sdk generates a lot of these
)

# generate PIO headers (has to be after SDK init)
pico_generate_pio_header(BlitHalPico ${CMAKE_CURRENT_LIST_DIR}/dbi-spi.pio)
pico_generate_pio_header(BlitHalPico ${CMAKE_CURRENT_LIST_DIR}/dbi-8bit.pio)
pico_generate_pio_header(BlitHalPico ${CMAKE_CURRENT_LIST_DIR}/audio/i2s.pio)
pico_generate_pio_header(BlitHalPico ${CMAKE_CURRENT_LIST_DIR}/display/dpi.pio)
pico_generate_pio_header(BlitHalPico ${CMAKE_CURRENT_LIST_DIR}/spi.pio)

# include picovision drivers
if(BLIT_DISPLAY_DRIVER STREQUAL "picovision")
    include(picovision_import.cmake)
endif()

# driver sources
target_sources(BlitHalPico INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/audio/${BLIT_AUDIO_DRIVER}.cpp
    ${CMAKE_CURRENT_LIST_DIR}/display/${BLIT_DISPLAY_DRIVER}.cpp
    ${CMAKE_CURRENT_LIST_DIR}/input/${BLIT_INPUT_DRIVER}.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/${BLIT_STORAGE_DRIVER}.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb/${BLIT_USB_DRIVER}.cpp
)

if(BLIT_ENABLE_CORE1)
    list(APPEND BLIT_BOARD_DEFINITIONS ENABLE_CORE1)
endif()

target_compile_definitions(BlitHalPico INTERFACE ${BLIT_BOARD_DEFINITIONS})
target_link_libraries(BlitHalPico INTERFACE ${BLIT_BOARD_LIBRARIES})

# for enabling stdio_usb
set(BLIT_USB_DRIVER ${BLIT_USB_DRIVER} PARENT_SCOPE)
set(BLIT_BOARD_DEFINITIONS ${BLIT_BOARD_DEFINITIONS} PARENT_SCOPE)

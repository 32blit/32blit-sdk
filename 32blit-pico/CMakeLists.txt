set(CMAKE_C_STANDARD 11)

# Initialise the Pico SDK
include (pico_sdk_import.cmake)

set(32BLIT_PICO 1 PARENT_SCOPE)

# prevent find_package errors in pico_add_uf2_output later
set(PICO_SDK_VERSION_MAJOR ${PICO_SDK_VERSION_MAJOR} PARENT_SCOPE)
set(PICO_SDK_VERSION_MINOR ${PICO_SDK_VERSION_MINOR} PARENT_SCOPE)
set(PICO_SDK_VERSION_REVISION ${PICO_SDK_VERSION_REVISION} PARENT_SCOPE)

# driver helper
# can override driver choice by pre-setting BLIT_x_DRIVER
function(blit_driver DRV NAME)
    set(var BLIT_${DRV}_DRIVER)
    string(TOUPPER ${var} var)

    if(NOT ${var})
        set(${var} ${NAME} PARENT_SCOPE)
    endif()
endfunction()

add_library(BlitHalPico INTERFACE)
target_sources(BlitHalPico INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/../3rd-party/fatfs/ff.c
    ${CMAKE_CURRENT_LIST_DIR}/../3rd-party/fatfs/ffunicode.c

    ${CMAKE_CURRENT_LIST_DIR}/display.cpp
    ${CMAKE_CURRENT_LIST_DIR}/file.cpp
    ${CMAKE_CURRENT_LIST_DIR}/led.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/st7789.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
)

target_link_libraries(BlitHalPico INTERFACE hardware_dma hardware_pio hardware_pwm hardware_spi pico_multicore pico_stdlib pico_unique_id tinyusb_device)
target_include_directories(BlitHalPico INTERFACE
    ${CMAKE_CURRENT_LIST_DIR} # for tusb_config
    ${CMAKE_CURRENT_LIST_DIR}/../3rd-party/fatfs
)

target_compile_definitions(BlitHalPico INTERFACE
    PICO_AUDIO_I2S_MONO_INPUT=1
    PICO_AUDIO_DMA_IRQ=1
)

target_compile_options(BlitHalPico INTERFACE
    -Wno-ignored-qualifiers # pico-sdk generates a lot of these
)

if(DEFINED PICO_ADDON)
    # for boards that don't have a board in the pico sdk
    # (usually because they are an add-on for a regular pico)
    set(CONFIG_PATH ${CMAKE_CURRENT_LIST_DIR}/board/${PICO_ADDON}/config.cmake)
else()
    set(CONFIG_PATH ${CMAKE_CURRENT_LIST_DIR}/board/${PICO_BOARD}/config.cmake)
endif()

if(EXISTS ${CONFIG_PATH})
    include(${CONFIG_PATH})
    message("Using board config \"${BLIT_BOARD_NAME}\"")
else()
    include(${CMAKE_CURRENT_LIST_DIR}/board/pico/config.cmake)
    if(DEFINED PICO_ADDON)
        message(WARNING "Using default config for \"${PICO_BOARD}\", add-on \"${PICO_ADDON}\"...")
    else()
        message(WARNING "Using default config for \"${PICO_BOARD}\"...")
    endif()
endif()

# default drivers
if(NOT BLIT_AUDIO_DRIVER)
    set(BLIT_AUDIO_DRIVER "none")
endif()
if(NOT BLIT_DISPLAY_DRIVER)
    set(BLIT_DISPLAY_DRIVER "none")
endif()
if(NOT BLIT_INPUT_DRIVER)
    set(BLIT_INPUT_DRIVER "none")
endif()
if(NOT BLIT_USB_DRIVER)
    set(BLIT_USB_DRIVER "device")
endif()

# driver dependencies
if(BLIT_AUDIO_DRIVER STREQUAL "i2s")
    set(BLIT_REQUIRE_PICO_EXTRAS TRUE)
    list(APPEND BLIT_BOARD_LIBRARIES pico_audio_i2s)
elseif(BLIT_AUDIO_DRIVER STREQUAL "pwm")
    set(BLIT_REQUIRE_PICO_EXTRAS TRUE)
    list(APPEND BLIT_BOARD_LIBRARIES pico_audio_pwm)
endif()

if(BLIT_DISPLAY_DRIVER STREQUAL "scanvideo")
    set(BLIT_REQUIRE_PICO_EXTRAS TRUE)
    set(BLIT_ENABLE_CORE1 TRUE)
    list(APPEND BLIT_BOARD_LIBRARIES pico_scanvideo_dpi)
elseif(BLIT_DISPLAY_DRIVER STREQUAL "st7789")
    list(APPEND BLIT_BOARD_DEFINITIONS DISPLAY_ST7789) # config defaults use this, also some games are using it for picosystem detection
endif()

if(BLIT_USB_DRIVER STREQUAL "host")
    list(APPEND BLIT_BOARD_DEFINITIONS USB_HOST)
    list(APPEND BLIT_BOARD_LIBRARIES tinyusb_host)
endif()

# late SDK init
# (pico_sdk_init needs to be after importing extras, which we don't know if we'll need until now)
if(BLIT_REQUIRE_PICO_EXTRAS)
    include(pico_extras_import.cmake)
endif()

pico_sdk_init()

# generate PIO headers (has to be after SDK init)
pico_generate_pio_header(BlitHalPico ${CMAKE_CURRENT_LIST_DIR}/st7789.pio)

# driver sources
target_sources(BlitHalPico INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/audio_${BLIT_AUDIO_DRIVER}.cpp
    ${CMAKE_CURRENT_LIST_DIR}/display_${BLIT_DISPLAY_DRIVER}.cpp
    ${CMAKE_CURRENT_LIST_DIR}/input_${BLIT_INPUT_DRIVER}.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb_${BLIT_USB_DRIVER}.cpp
)

if(BLIT_ENABLE_CORE1)
    list(APPEND BLIT_BOARD_DEFINITIONS ENABLE_CORE1)
endif()

target_compile_definitions(BlitHalPico INTERFACE ${BLIT_BOARD_DEFINITIONS})
target_link_libraries(BlitHalPico INTERFACE ${BLIT_BOARD_LIBRARIES})

# functions
function(blit_executable_common NAME)
    target_link_libraries(${NAME} BlitEngine)

endfunction()

function(blit_executable_int_flash NAME SOURCES)
    add_executable(${NAME} ${SOURCES} ${ARGN})
    target_link_libraries(${NAME} BlitHalPico BlitEngine)

    pico_enable_stdio_uart(${NAME} 1)
    pico_enable_stdio_usb(${NAME} 0)

    pico_add_extra_outputs(${NAME})

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
        DESTINATION bin
    )
endfunction()

function(blit_executable NAME SOURCES)
    blit_executable_int_flash(${NAME} ${SOURCES} ${ARGN})
endfunction()

function(blit_metadata TARGET FILE)
    # cause cmake to reconfigure whenever the asset list changes
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})

    # get the inputs/outputs for the asset tool (at configure time)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -m ttblit cmake --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --cmake ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)

    # create metadata/binary info source at build time
    set(METADATA_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_binary_info.cpp")

    add_custom_command(
        OUTPUT ${METADATA_SOURCE}
        COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${PYTHON_EXECUTABLE} -m ttblit metadata --force --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --pico-bi ${METADATA_SOURCE}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
    )

    # add the generated source
    target_sources(${TARGET} PRIVATE ${METADATA_SOURCE})

endfunction()

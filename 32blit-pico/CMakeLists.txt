cmake_policy(SET CMP0079 NEW) # target_link_libraries() allows use with targets in other directories.

set(CMAKE_C_STANDARD 11)

# Initialise the Pico SDK
set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/board)
include (pico_sdk_import.cmake)

set(32BLIT_PICO 1 PARENT_SCOPE)

# prevent find_package errors in pico_add_uf2_output later
set(PICO_SDK_VERSION_MAJOR ${PICO_SDK_VERSION_MAJOR} PARENT_SCOPE)
set(PICO_SDK_VERSION_MINOR ${PICO_SDK_VERSION_MINOR} PARENT_SCOPE)
set(PICO_SDK_VERSION_REVISION ${PICO_SDK_VERSION_REVISION} PARENT_SCOPE)

# make sure BlitEngine is built with the right exception flags
target_link_libraries(BlitEngine PUBLIC pico_cxx_options pico_base_headers)

# also enable function/data sectons
target_compile_options(BlitEngine PRIVATE -ffunction-sections -fdata-sections)

add_subdirectory(../32blit-shared ../32blit-shared)

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
    ${CMAKE_CURRENT_LIST_DIR}/display.cpp
    ${CMAKE_CURRENT_LIST_DIR}/file.cpp
    ${CMAKE_CURRENT_LIST_DIR}/led.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/multiplayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
)

target_link_libraries(BlitHalPico INTERFACE
    hardware_dma hardware_i2c hardware_pio hardware_pwm hardware_spi
    pico_multicore pico_stdlib pico_unique_id pico_rand
    tinyusb_device
    FatFsBlitAPI
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

if(DEFINED PICO_ADDON)
    # for boards that don't have a board in the pico sdk
    # (usually because they are an add-on for a regular pico)
    set(CONFIG_PATH ${CMAKE_CURRENT_LIST_DIR}/board/${PICO_ADDON})
    set(BOARD_ID ${PICO_ADDON})
else()
    set(CONFIG_PATH ${CMAKE_CURRENT_LIST_DIR}/board/${PICO_BOARD})
    set(BOARD_ID ${PICO_BOARD})
endif()

if(NOT EXISTS ${CONFIG_PATH}/config.cmake)
    set(CONFIG_PATH ${CMAKE_CURRENT_LIST_DIR}/board/pico)
    if(DEFINED PICO_ADDON)
        message(WARNING "Using default config for \"${PICO_BOARD}\", add-on \"${PICO_ADDON}\"...")
    else()
        message(WARNING "Using default config for \"${PICO_BOARD}\"...")
    endif()
endif()

include(${CONFIG_PATH}/config.cmake)
message("Using board config \"${BLIT_BOARD_NAME}\"")

if(EXISTS ${CONFIG_PATH}/config.h)
    list(APPEND BLIT_BOARD_DEFINITIONS "BLIT_BOARD_CONFIG=\"${CONFIG_PATH}/config.h\"")
endif()

# board id definition
string(TOUPPER ${BOARD_ID} BOARD_ID)
list(APPEND BLIT_BOARD_DEFINITIONS BLIT_BOARD_${BOARD_ID})

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
if(NOT BLIT_STORAGE_DRIVER)
    set(BLIT_STORAGE_DRIVER "flash")
endif()
if(NOT BLIT_USB_DRIVER)
    set(BLIT_USB_DRIVER "device")
endif()

# driver dependencies
if(BLIT_AUDIO_DRIVER STREQUAL "pwm")
    set(BLIT_REQUIRE_PICO_EXTRAS TRUE)
    list(APPEND BLIT_BOARD_LIBRARIES pico_audio_pwm)
endif()

if(BLIT_DISPLAY_DRIVER STREQUAL "picovision")
    list(APPEND BLIT_BOARD_LIBRARIES aps6404 swd_load)
elseif(BLIT_DISPLAY_DRIVER STREQUAL "scanvideo")
    set(BLIT_REQUIRE_PICO_EXTRAS TRUE)
    set(BLIT_ENABLE_CORE1 TRUE)
    list(APPEND BLIT_BOARD_LIBRARIES pico_scanvideo_dpi)
endif()

if(BLIT_INPUT_DRIVER STREQUAL "usb_hid")
    list(APPEND BLIT_BOARD_DEFINITIONS INPUT_USB_HID)
endif()

if(BLIT_STORAGE_DRIVER STREQUAL "sd_spi")
    list(APPEND BLIT_BOARD_DEFINITIONS STORAGE_SD)
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

# some file paths we need later
set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/memmap_blit.ld PARENT_SCOPE)
set(STARTUP_SRC ${CMAKE_CURRENT_LIST_DIR}/startup.S PARENT_SCOPE)

# functions
function(blit_executable NAME)
    message(STATUS "Processing ${NAME}")
    blit_executable_args(${ARGN})

    add_library(${NAME} STATIC ${SOURCES})
    target_compile_definitions(${NAME} PRIVATE PICO_EMBED_XIP_SETUP=1)
    target_link_libraries(${NAME} BlitHalPico BlitEngine)

    # standalone .uf2
    add_executable(${NAME}-standalone)
    target_link_libraries(${NAME}-standalone ${NAME})

    pico_enable_stdio_uart(${NAME}-standalone 1)
    if(NOT BLIT_USB_DRIVER STREQUAL "host")
        pico_enable_stdio_usb(${NAME}-standalone 1)
        # re-enable baud rate reset (we don't define our own tud_cdc_line_coding_cb)
        # we have to set all the defines to compile? (something broken with the defaults?)
        target_compile_definitions(${NAME}-standalone PRIVATE PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1 PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE=1200 PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK=0)
    endif()

    pico_add_extra_outputs(${NAME}-standalone)

    # loadable .blit
    add_executable(${NAME}-bl)
    target_compile_definitions(${NAME}-bl PRIVATE ${BLIT_BOARD_DEFINITIONS}) # need these for framebuffer config
    target_link_libraries(${NAME}-bl ${NAME})

    pico_set_linker_script(${NAME}-bl ${LINKER_SCRIPT})
    target_sources(${NAME}-bl PRIVATE ${STARTUP_SRC})

    pico_enable_stdio_uart(${NAME}-bl 1)
    pico_enable_stdio_usb(${NAME}-bl 0)

    pico_add_bin_output(${NAME}-bl)

    # Ideally we want the .blit filename to match the .elf, but TARGET_FILE_BASE_NAME isn't always available
	# (This only affects the firmware updater as it's the only thing setting a custom OUTPUT_NAME)
	if(${CMAKE_VERSION} VERSION_LESS "3.15.0")
        set(BLIT_FILENAME ${NAME}.blit)
    else()
        set(BLIT_FILENAME $<TARGET_FILE_BASE_NAME:${NAME}>.blit)
    endif()

    # no relocs, just copy it
    set(BIN_NAME "$<IF:$<BOOL:$<TARGET_PROPERTY:${NAME}-bl,OUTPUT_NAME>>,$<TARGET_PROPERTY:${NAME}-bl,OUTPUT_NAME>,$<TARGET_PROPERTY:${NAME}-bl,NAME>>.bin")
    add_custom_command(TARGET ${NAME}-bl POST_BUILD
        COMMENT "Building ${NAME}.blit"
        COMMAND cp ${BIN_NAME} ${BLIT_FILENAME}
    )

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-standalone.uf2
        DESTINATION bin
    )
endfunction()

function(blit_metadata TARGET FILE)
    if(NOT IS_ABSOLUTE ${FILE})
        set(FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})
    endif()

    # cause cmake to reconfigure whenever the asset list changes
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${FILE})

    # get the inputs/outputs for the asset tool (at configure time)
    execute_process(
        COMMAND ${32BLIT_TOOLS_EXECUTABLE} cmake --config ${FILE} --cmake ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake
        RESULT_VARIABLE TOOL_RESULT
    )
    if(${TOOL_RESULT})
        message(FATAL_ERROR "Reading metadata config failed!\n")
    endif()

    include(${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)

    # create metadata/binary info source at build time
    set(METADATA_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_binary_info.cpp")

    add_custom_command(
        OUTPUT ${METADATA_SOURCE}
        COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${32BLIT_TOOLS_EXECUTABLE} metadata --force --config ${FILE} --pico-bi ${METADATA_SOURCE}
        DEPENDS ${FILE}
        VERBATIM
    )

    # add the generated source
    target_sources(${TARGET}-standalone PRIVATE ${METADATA_SOURCE})

    # real .blit metadata
	if(${CMAKE_VERSION} VERSION_LESS "3.15.0")
        set(BLIT_FILENAME ${TARGET}.blit)
    else()
        set(BLIT_FILENAME $<TARGET_FILE_BASE_NAME:${TARGET}>.blit)
    endif()

    add_custom_command(
        TARGET ${TARGET}-bl POST_BUILD
        COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${32BLIT_TOOLS_EXECUTABLE} metadata --config ${FILE} --file ${CMAKE_CURRENT_BINARY_DIR}/${BLIT_FILENAME}
    )

    # force relink on change so that the post-build commands are rerun
    set_property(TARGET ${TARGET}-bl APPEND PROPERTY LINK_DEPENDS ${FILE} ${METADATA_DEPENDS})

    # avoid the fallback to target name
    target_compile_definitions(${TARGET} PRIVATE PICO_NO_BI_PROGRAM_NAME=1)

endfunction()

cmake_policy(SET CMP0079 NEW) # target_link_libraries() allows use with targets in other directories.

set(CMAKE_C_STANDARD 11)

# Initialise the Pico SDK
set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/board)
include (pico_sdk_import.cmake)

set(32BLIT_PICO 1 PARENT_SCOPE)

if(PICO_PLATFORM MATCHES "^rp2350")
    set(32BLIT_PICO_2350 1)
    set(32BLIT_PICO_2350 ${32BLIT_PICO_2350} PARENT_SCOPE)
endif()

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
    ${CMAKE_CURRENT_LIST_DIR}/blit_launch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/display.cpp
    ${CMAKE_CURRENT_LIST_DIR}/file.cpp
    ${CMAKE_CURRENT_LIST_DIR}/led.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/multiplayer.cpp
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
if(32BLIT_PICO_2350)
    set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/memmap_blit_2350.ld.in PARENT_SCOPE)
else()
    set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/memmap_blit.ld.in PARENT_SCOPE)
endif()
set(STARTUP_SRC ${CMAKE_CURRENT_LIST_DIR}/startup.S ${CMAKE_CURRENT_LIST_DIR}/startup.cpp PARENT_SCOPE)
set(LAUNCHERSHARED_DIR ${CMAKE_CURRENT_LIST_DIR}/../launcher-shared PARENT_SCOPE)

# functions
function(blit_executable NAME)
    message(STATUS "Processing ${NAME}")
    blit_executable_args(${ARGN})

    add_executable(${NAME} ${SOURCES})
    target_link_libraries(${NAME} BlitEngine pico_platform_headers)

    if(PICO_STANDALONE_UF2)
        # standalone .uf2
        # TODO: import pico hal if standalone build
        if(NOT TARGET LauncherShared)
            add_subdirectory(${LAUNCHERSHARED_DIR} launcher-shared)
        endif()

        pico_enable_stdio_uart(${NAME} 1)
        if(NOT BLIT_USB_DRIVER STREQUAL "host")
            pico_enable_stdio_usb(${NAME} 1)
            # re-enable baud rate reset (we don't define our own tud_cdc_line_coding_cb)
            # we have to set all the defines to compile? (something broken with the defaults?)
            target_compile_definitions(${NAME} PRIVATE PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1 PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE=1200 PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK=0)
        endif()

        target_compile_definitions(${NAME} PRIVATE BLIT_PICO_STANDALONE=1 PICO_EMBED_XIP_SETUP=1)
        target_link_libraries(${NAME} BlitHalPico)

        pico_add_extra_outputs(${NAME})

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
            DESTINATION bin
        )
    elseif(PICO_BLIT)
        # loadable .blit
        set_property(TARGET ${NAME} PROPERTY BLIT_PICO_BLIT TRUE)

        # calculate and verify flash offset
        if(NOT PICO_BLIT_OFFSET_KB)
            if(32BLIT_PICO_2350)
                # if we build the file for a 4MB offset, we can use address translation
                set(PICO_BLIT_OFFSET_KB 4096)
            else()
                set(PICO_BLIT_OFFSET_KB 512)
            endif()
        endif()

        math(EXPR offset_mod "${PICO_BLIT_OFFSET_KB} % 64")
        if(${PICO_BLIT_OFFSET_KB} LESS 256 OR offset_mod)
            message(FATAL_ERROR "Blit offset should be >= 256k and a multiple of 64k")
        endif()

        math(EXPR FLASH_OFFSET_BYTES "${PICO_BLIT_OFFSET_KB} * 1024")

        # customise linker script
        configure_file(${LINKER_SCRIPT} memmap_blit.ld)
        set(LINKER_SCRIPT_OUT ${CMAKE_CURRENT_BINARY_DIR}/memmap_blit.ld)

        target_compile_definitions(${NAME} PRIVATE ${BLIT_BOARD_DEFINITIONS}) # need these for framebuffer config
        target_compile_options(${NAME} PRIVATE -ffunction-sections -fdata-sections)

        # set device id in header, these should match the BlitDevice enum
        if(32BLIT_PICO_2350)
            set_source_files_properties(${STARTUP_SRC} PROPERTIES COMPILE_DEFINITIONS DEVICE_ID=3)
        else()
            set_source_files_properties(${STARTUP_SRC} PROPERTIES COMPILE_DEFINITIONS DEVICE_ID=2)
        endif()

        # minimal pico-sdk libs
        target_link_libraries(${NAME} boot_picobin_headers pico_bit_ops pico_clib_interface pico_cxx_options pico_divider pico_double pico_int64_ops pico_float pico_malloc pico_mem_ops pico_runtime_headers)
        target_compile_definitions(${NAME} PRIVATE PICO_TIME_DEFAULT_ALARM_POOL_DISABLED) # avoid pulling timer and irq code

        target_link_options(${NAME} PRIVATE --specs=nosys.specs LINKER:--script=${LINKER_SCRIPT_OUT} LINKER:--gc-sections)
        set_property(TARGET ${NAME} APPEND PROPERTY LINK_DEPENDS ${LINKER_SCRIPT_OUT})

        target_sources(${NAME} PRIVATE ${STARTUP_SRC})

        pico_add_bin_output(${NAME})

        # Ideally we want the .blit filename to match the .elf, but TARGET_FILE_BASE_NAME isn't always available
        # (This only affects the firmware updater as it's the only thing setting a custom OUTPUT_NAME)
        if(${CMAKE_VERSION} VERSION_LESS "3.15.0")
            set(BLIT_FILENAME ${NAME}.blit)
        else()
            set(BLIT_FILENAME $<TARGET_FILE_BASE_NAME:${NAME}>.blit)
        endif()

        # no relocs, just copy it
        set(BIN_NAME "$<IF:$<BOOL:$<TARGET_PROPERTY:${NAME},OUTPUT_NAME>>,$<TARGET_PROPERTY:${NAME},OUTPUT_NAME>,$<TARGET_PROPERTY:${NAME},NAME>>.bin")
        add_custom_command(TARGET ${NAME} POST_BUILD
            VERBATIM
            COMMENT "Building ${NAME}.blit"
            COMMAND cp ${BIN_NAME} ${BLIT_FILENAME}
        )

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BLIT_FILENAME}
            DESTINATION bin
        )
    endif()
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

    get_property(PICO_BLIT TARGET ${TARGET} PROPERTY BLIT_PICO_BLIT)

    if(NOT PICO_BLIT)
        # create metadata/binary info source at build time
        set(METADATA_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_binary_info.cpp")

        add_custom_command(
            OUTPUT ${METADATA_SOURCE}
            COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${32BLIT_TOOLS_EXECUTABLE} metadata --force --config ${FILE} --pico-bi ${METADATA_SOURCE}
            DEPENDS ${FILE}
            VERBATIM
        )

        # add the generated source
        target_sources(${TARGET} PRIVATE ${METADATA_SOURCE})

        # avoid the fallback to target name
        target_compile_definitions(${TARGET} PRIVATE PICO_NO_BI_PROGRAM_NAME=1)
    else()
        # real .blit metadata
        if(${CMAKE_VERSION} VERSION_LESS "3.15.0")
            set(BLIT_FILENAME ${TARGET}.blit)
        else()
            set(BLIT_FILENAME $<TARGET_FILE_BASE_NAME:${TARGET}>.blit)
        endif()

        add_custom_command(
            TARGET ${TARGET} POST_BUILD
            COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${32BLIT_TOOLS_EXECUTABLE} metadata --config ${FILE} --file ${CMAKE_CURRENT_BINARY_DIR}/${BLIT_FILENAME}
            VERBATIM
        )

        # force relink on change so that the post-build commands are rerun
        set_property(TARGET ${TARGET} APPEND PROPERTY LINK_DEPENDS ${FILE} ${METADATA_DEPENDS})
    endif()

endfunction()

add_subdirectory(loader)

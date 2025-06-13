# driver helper
# can override driver choice by pre-setting BLIT_x_DRIVER
function(blit_driver DRV NAME)
    set(var BLIT_${DRV}_DRIVER)
    string(TOUPPER ${var} var)

    if(NOT ${var})
        set(${var} ${NAME} PARENT_SCOPE)
    endif()
endfunction()

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

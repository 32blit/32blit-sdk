# This file can be dropped into a project to help locate the Picovision library
# It will also set up the required include and module search paths.

if (NOT PIMORONI_PICOVISION_PATH)
    if(PICO_SDK_PATH AND EXISTS "${PICO_SDK_PATH}/../picovision")
        set(PIMORONI_PICOVISION_PATH ${PICO_SDK_PATH}/../picovision)
        message("Defaulting PIMORONI_PICOVISION_PATH as sibling of PICO_SDK_PATH: ${PIMORONI_PICO_PATH}")
    else()
        set(PIMORONI_PICOVISION_PATH "../../picovision/")
    endif()
endif()

if(NOT IS_ABSOLUTE ${PIMORONI_PICOVISION_PATH})
    get_filename_component(
        PIMORONI_PICOVISION_PATH
        "${CMAKE_CURRENT_BINARY_DIR}/${PIMORONI_PICOVISION_PATH}"
        ABSOLUTE)
endif()

if (NOT EXISTS ${PIMORONI_PICOVISION_PATH})
    message(FATAL_ERROR "Directory '${PIMORONI_PICOVISION_PATH}' not found")
endif ()

if (NOT EXISTS ${PIMORONI_PICOVISION_PATH}/picovision_import.cmake)
    message(FATAL_ERROR "Directory '${PIMORONI_PICOVISION_PATH}' does not appear to contain the Picovision library")
endif ()

message("PIMORONI_PICOVISION_PATH is ${PIMORONI_PICOVISION_PATH}")

set(PIMORONI_PICOVISION_PATH ${PIMORONI_PICOVISION_PATH} CACHE PATH "Path to the Picovision libraries" FORCE)

list(APPEND CMAKE_MODULE_PATH ${PIMORONI_PICOVISION_PATH})

include(picovision)

if(PICO_BOARD)
    set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/board)
    include(${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake)
endif()

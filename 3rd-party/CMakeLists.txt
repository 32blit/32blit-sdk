add_library(FatFs INTERFACE)
target_sources(FatFs INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/fatfs/ff.c
    ${CMAKE_CURRENT_LIST_DIR}/fatfs/ffsystem.c
    ${CMAKE_CURRENT_LIST_DIR}/fatfs/ffunicode.c
)
target_include_directories(FatFs INTERFACE ${CMAKE_CURRENT_LIST_DIR}/fatfs)

add_library(MiniMP3 INTERFACE)
target_include_directories(MiniMP3 INTERFACE ${CMAKE_CURRENT_LIST_DIR})
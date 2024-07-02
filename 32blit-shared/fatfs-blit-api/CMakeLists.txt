add_library(FatFsBlitAPI INTERFACE)
target_sources(FatFsBlitAPI INTERFACE ${CMAKE_CURRENT_LIST_DIR}/fatfs_blit_api.cpp)
target_include_directories(FatFsBlitAPI INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(FatFsBlitAPI INTERFACE FatFs)
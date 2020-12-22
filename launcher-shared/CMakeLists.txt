

add_library(LauncherShared metadata.cpp)
target_link_libraries(LauncherShared BlitEngine)
target_include_directories(LauncherShared PUBLIC ${CMAKE_CURRENT_LIST_DIR})
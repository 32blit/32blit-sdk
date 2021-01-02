

add_library(LauncherShared metadata.cpp)
target_link_libraries(LauncherShared BlitEngine)
target_include_directories(LauncherShared PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if(${CMAKE_SYSTEM_NAME} STREQUAL Generic)
	set_target_properties(LauncherShared PROPERTIES COMPILE_FLAGS "-fPIC -mno-pic-data-is-text-relative -mno-single-pic-base")
endif()

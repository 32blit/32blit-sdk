find_package(SDL2 REQUIRED)
include_directories(${BLIT_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
add_library(BlitHalSDL STATIC
	${CMAKE_CURRENT_LIST_DIR}/Input.cpp
	${CMAKE_CURRENT_LIST_DIR}/Main.cpp
	${CMAKE_CURRENT_LIST_DIR}/Renderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/System.cpp
)
add_definitions(-DWINDOW_NAME=\"CMake!\" -DNO_FFMPEG_CAPTURE)
list(APPEND BLIT_LIBRARIES BlitHalSDL BlitEngine ${SDL2_LIBRARIES})

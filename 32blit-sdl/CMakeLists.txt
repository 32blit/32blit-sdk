find_package(SDL2 REQUIRED)

add_library(BlitHalSDL STATIC
	Input.cpp
	Main.cpp
	Renderer.cpp
	System.cpp
)

target_include_directories(BlitHalSDL
	PRIVATE
		${SDL2_INCLUDE_DIRS}
)

target_link_libraries(BlitHalSDL BlitEngine ${SDL2_LIBRARIES})

add_definitions(-DWINDOW_TITLE=\"CMake!\" -DNO_FFMPEG_CAPTURE)

set(BLIT_HAL BlitHalSDL PARENT_SCOPE)

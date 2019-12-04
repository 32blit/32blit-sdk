find_package(SDL2 REQUIRED)

add_library(BlitHalSDL STATIC
	Input.cpp
	Main.cpp
	Renderer.cpp
	System.cpp
)

target_include_directories(BlitHalSDL
	PRIVATE	${SDL2_INCLUDE_DIRS}
)

target_link_libraries(BlitHalSDL BlitEngine ${SDL2_LIBRARIES})

target_compile_definitions(BlitHalSDL
	PUBLIC
		-DWINDOW_TITLE=\"CMake!\"
		-DNO_FFMPEG_CAPTURE
)

function(blit_executable NAME SOURCES)
	add_executable(${NAME} ${SOURCES} ${ARGN})
  	target_link_libraries(${NAME} BlitHalSDL BlitEngine)
endfunction()

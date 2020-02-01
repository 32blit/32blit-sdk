add_library(BlitHalSDL STATIC
	Input.cpp
	Main.cpp
	Renderer.cpp
	System.cpp
)

if(EMSCRIPTEN)
	target_compile_options(BlitHalSDL
		PRIVATE
			"-s;USE_SDL=2"
	)

	set(SDL2_LIBRARIES "-s USE_SDL=2")
else()
	find_package(SDL2 REQUIRED)

	# If SDL2 was built using CMake, the generated configuration files define SDL2::* targets instead of the SDL2_* variables
	if(TARGET SDL2::SDL2)
		set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
	else()
		target_include_directories(BlitHalSDL
			PRIVATE	${SDL2_INCLUDE_DIRS}
		)
	endif()
endif()

target_link_libraries(BlitHalSDL PUBLIC BlitEngine ${SDL2_LIBRARIES})

target_compile_definitions(BlitHalSDL
	PRIVATE
		-DWINDOW_TITLE=\"CMake!\"
)

if(DEFINED VIDEO_CAPTURE AND VIDEO_CAPTURE)
	find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
	find_library(AVCODEC_LIBRARY avcodec)

	find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
	find_library(AVFORMAT_LIBRARY avformat)

	find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
	find_library(AVUTIL_LIBRARY avutil)

	find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
	find_library(AVDEVICE_LIBRARY avdevice)

	find_library(SWRESAMPLE_LIBRARY swresample)

	find_library(SWSCALE_LIBRARY swscale)

	target_include_directories(BlitHalSDL PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR})
	target_link_libraries(BlitHalSDL PUBLIC ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY} ${SWRESAMPLE_LIBRARY} ${SWSCALE_LIBRARY})

	target_sources(BlitHalSDL PRIVATE VideoCapture.cpp VideoCaptureFfmpeg.cpp)

	target_compile_definitions(BlitHalSDL
		PRIVATE
			-DVIDEO_CAPTURE
			-D__STDC_CONSTANT_MACROS
	)
endif()


function(blit_executable NAME SOURCES)
	add_executable(${NAME} ${SOURCES} ${ARGN})

	if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
		target_link_libraries(${NAME} BlitHalSDL)
	elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
  		target_link_libraries(${NAME} -Wl,-force_load BlitHalSDL)
	else()
  		target_link_libraries(${NAME} -Wl,--whole-archive BlitHalSDL -Wl,--no-whole-archive)
	endif()

	if(EMSCRIPTEN)
		set_target_properties(${NAME} PROPERTIES SUFFIX ".html")
	endif()
endfunction()

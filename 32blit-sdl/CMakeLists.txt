add_library(BlitHalSDL STATIC
	File.cpp
	Input.cpp
	JPEG.cpp
	Main.cpp
	Renderer.cpp
	Audio.cpp
	System.cpp
)

if(EMSCRIPTEN)
	target_compile_options(BlitHalSDL
		PRIVATE
			"-s;USE_SDL=2;-s ;USE_SDL_IMAGE=2"
	)

	set(SDL2_LIBRARIES "-s USE_SDL=2")
	set(SDL2_IMAGE_LIBRARY "-s USE_SDL_IMAGE=2")
else()
	# attempt to find the framework (find_package won't work as there's no config file)
	if(APPLE)
		find_library(SDL2_LIBRARIES NAMES SDL2)
		
		find_path(SDL2_INCLUDE_DIRS NAMES SDL.h
			PATHS ~/Library/Frameworks /Library/Frameworks
		)
	endif()

	if(NOT SDL2_LIBRARIES OR NOT SDL2_INCLUDE_DIRS)
		find_package(SDL2 REQUIRED)
	endif()

	# If SDL2 was built using CMake, the generated configuration files define SDL2::* targets instead of the SDL2_* variables
	if(TARGET SDL2::SDL2)
		set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
	# handle SDL2 built with only a static library
	elseif(TARGET SDL2::SDL2-static)
		set(SDL2_LIBRARIES SDL2::SDL2-static SDL2::SDL2main)
	else()
		target_include_directories(BlitHalSDL
			PRIVATE	${SDL2_INCLUDE_DIRS}
		)
	endif()

	# find SDL2_image
	# TODO: may need to be more complicated
	find_path(SDL2_IMAGE_INCLUDE_DIR SDL_image.h
		HINTS ${SDL2_DIR} ${SDL2_DIR}../../../
		PATH_SUFFIXES SDL2 include/SDL2 include
	)
	target_include_directories(BlitHalSDL
		PRIVATE	${SDL2_IMAGE_INCLUDE_DIR}
	)

	# vs
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(VC_LIB_PATH_SUFFIX lib/x64)
	else()
		set(VC_LIB_PATH_SUFFIX lib/x86)
	endif()

	find_library(SDL2_IMAGE_LIBRARY
		NAMES SDL2_image
		HINTS ${SDL2_DIR} ${SDL2_DIR}../../../
		PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
	)

	# dlls
	find_file(SDL2_IMAGE_DLL SDL2_image.dll
		HINTS ${SDL2_DIR} ${SDL2_DIR}../../../
		PATH_SUFFIXES bin ${VC_LIB_PATH_SUFFIX}
	)
endif()

target_link_libraries(BlitHalSDL PUBLIC BlitEngine ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARY})

# copy SDL2.dll to build/install dir for windows users
if(SDL2_DLL)
	configure_file(${SDL2_DLL} ${CMAKE_BINARY_DIR}/SDL2.dll COPYONLY)
	install(FILES ${SDL2_DLL} DESTINATION bin)
endif()

if(SDL2_IMAGE_DLL)
	configure_file(${SDL2_IMAGE_DLL} ${CMAKE_BINARY_DIR}/SDL2_image.dll COPYONLY)
	install(FILES ${SDL2_IMAGE_DLL} DESTINATION bin)
endif()

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

	find_library(SWRESAMPLE_LIBRARY swresample)

	find_library(SWSCALE_LIBRARY swscale)

	target_include_directories(BlitHalSDL PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR})
	target_link_libraries(BlitHalSDL PUBLIC ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWRESAMPLE_LIBRARY} ${SWSCALE_LIBRARY})

	target_sources(BlitHalSDL PRIVATE VideoCapture.cpp VideoCaptureFfmpeg.cpp)

	target_compile_definitions(BlitHalSDL
		PRIVATE
			-DVIDEO_CAPTURE
			-D__STDC_CONSTANT_MACROS
	)
endif()

function(blit_executable NAME SOURCES)
	add_executable(${NAME} ${SOURCES} ${ARGN})

	install(TARGETS ${NAME}
		RUNTIME DESTINATION bin
	)

	if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
		target_link_libraries(${NAME} BlitHalSDL)
	elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
  		target_link_libraries(${NAME} -Wl,-force_load BlitHalSDL)
	else()
  		target_link_libraries(${NAME} -Wl,--whole-archive BlitHalSDL -Wl,--no-whole-archive)
	endif()

	if(EMSCRIPTEN)
		set_target_properties(${NAME} PROPERTIES
			SUFFIX ".html"
			LINK_FLAGS "-s ENVIRONMENT=web -s SDL2_IMAGE_FORMATS=['jpg']"
		)
	endif()
endfunction()

function(blit_executable_int_flash NAME SOURCES)
	blit_executable(${NAME} ${SOURCES} ${ARGN})
endfunction()

function(blit_metadata TARGET FILE)
	# do nothing
endfunction()

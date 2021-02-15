add_library(BlitHalSDL STATIC
	DefaultMetadata.cpp
	File.cpp
	Input.cpp
	JPEG.cpp
	Main.cpp
	Multiplayer.cpp
	Renderer.cpp
	Audio.cpp
	System.cpp
)

if(EMSCRIPTEN)
	target_compile_options(BlitHalSDL
		PRIVATE
			"-s;USE_SDL=2;-s ;USE_SDL_IMAGE=2;-s  ;USE_SDL_NET=2"
	)

	set(SDL2_LIBRARIES "-s USE_SDL=2")
	set(SDL2_IMAGE_LIBRARY "-s USE_SDL_IMAGE=2")
	set(SDL2_NET_LIBRARY "-s USE_SDL_NET=2")

	set(EMSCRIPTEN_SHELL ${CMAKE_CURRENT_SOURCE_DIR}/emscripten-shell.html PARENT_SCOPE)
else()
	# attempt to find the framework (find_package won't work as there's no config file)
	if(APPLE)
		find_library(SDL2_LIBRARIES NAMES SDL2)

		find_path(SDL2_INCLUDE_DIRS NAMES SDL.h
			PATHS ~/Library/Frameworks /Library/Frameworks
		)
	endif()

	# fallback guess for SDL location on Windows
	if(WIN32 AND NOT SDL2_DIR AND 32BLIT_DIR)
		set(SDL2_DIR "${32BLIT_DIR}/vs/sdl")
	endif()

	if(NOT SDL2_LIBRARIES OR NOT SDL2_INCLUDE_DIRS)
		find_package(SDL2 REQUIRED)
	endif()

	# If SDL2 was built using CMake, the generated configuration files define SDL2::* targets instead of the SDL2_* variables
	if(TARGET SDL2::SDL2)
		set(SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2)
	# handle SDL2 built with only a static library
	elseif(TARGET SDL2::SDL2-static)
		set(SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2-static)
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

	#SDL_net
	find_path(SDL2_NET_INCLUDE_DIR SDL_net.h
		HINTS ${SDL2_DIR} ${SDL2_DIR}../../../
		PATH_SUFFIXES SDL2 include/SDL2 include
	)

	find_library(SDL2_NET_LIBRARY
		NAMES SDL2_net
		HINTS ${SDL2_DIR} ${SDL2_DIR}../../../
		PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
	)

	# dlls
	find_file(SDL2_NET_DLL SDL2_net.dll
		HINTS ${SDL2_DIR} ${SDL2_DIR}../../../
		PATH_SUFFIXES bin ${VC_LIB_PATH_SUFFIX}
	)

	if(NOT SDL2_IMAGE_INCLUDE_DIR OR NOT SDL2_IMAGE_LIBRARY)
		message(FATAL_ERROR "SDL_image not found!")
	endif()

	if(NOT SDL2_NET_INCLUDE_DIR OR NOT SDL2_NET_LIBRARY)
		message(FATAL_ERROR "SDL_net not found!")
	endif()

	target_include_directories(BlitHalSDL
		PRIVATE	${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_NET_INCLUDE_DIR}
	)
endif()

target_link_libraries(BlitHalSDL PUBLIC BlitEngine ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARY} ${SDL2_NET_LIBRARY})

# copy SDL2 dlls to build/install dir for windows users
if(SDL2_DLL)
	install(FILES ${SDL2_DLL} DESTINATION bin)
	set(SDL2_DLL ${SDL2_DLL} PARENT_SCOPE)
endif()

if(SDL2_IMAGE_DLL)
	install(FILES ${SDL2_IMAGE_DLL} DESTINATION bin)
endif()

if(SDL2_NET_DLL)
	install(FILES ${SDL2_NET_DLL} DESTINATION bin)
endif()

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
	message(STATUS "Processing ${NAME}")
	add_executable(${NAME} MACOSX_BUNDLE ${SOURCES} ${ARGN})

	install(TARGETS ${NAME}
		RUNTIME DESTINATION bin
		BUNDLE DESTINATION bin
	)

	if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
		target_link_libraries(${NAME} BlitHalSDL)
	elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
  		target_link_libraries(${NAME} -Wl,-force_load BlitHalSDL)
	else()
  		target_link_libraries(${NAME} -Wl,--whole-archive BlitHalSDL -Wl,--no-whole-archive)
	endif()

    # windows dll fun
    if(SDL2_DLL)
        configure_file(${SDL2_DLL} ${CMAKE_CURRENT_BINARY_DIR}/SDL2.dll COPYONLY)
    endif()

    if(SDL2_IMAGE_DLL)
        configure_file(${SDL2_IMAGE_DLL} ${CMAKE_CURRENT_BINARY_DIR}/SDL2_image.dll COPYONLY)
	endif()

	if(SDL2_NET_DLL)
		configure_file(${SDL2_NET_DLL} ${CMAKE_CURRENT_BINARY_DIR}/SDL2_net.dll COPYONLY)
	endif()

	if(EMSCRIPTEN)
		set_target_properties(${NAME} PROPERTIES
			SUFFIX ".html"
			LINK_FLAGS "-s ENVIRONMENT=web -s SDL2_IMAGE_FORMATS=['jpg'] --shell-file ${EMSCRIPTEN_SHELL} -s 'EXTRA_EXPORTED_RUNTIME_METHODS=[\"addRunDependency\", \"removeRunDependency\"]'"
			LINK_DEPENDS ${EMSCRIPTEN_SHELL}
		)

		target_link_libraries(${NAME} "-lidbfs.js") # include the persistent IndexedDB-based filesystem

		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.js ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.wasm
			DESTINATION bin
		)
	endif()

	if(APPLE AND SDL2_LIBRARIES MATCHES "\.framework$")
	    # install the SDL frameworks
		install(DIRECTORY ${SDL2_LIBRARIES} DESTINATION "bin/$<TARGET_FILE_NAME:${NAME}>.app/Contents/Frameworks")
		install(DIRECTORY ${SDL2_IMAGE_LIBRARY} DESTINATION "bin/$<TARGET_FILE_NAME:${NAME}>.app/Contents/Frameworks")
		install(DIRECTORY ${SDL2_NET_LIBRARY} DESTINATION "bin/$<TARGET_FILE_NAME:${NAME}>.app/Contents/Frameworks")
	elseif(APPLE)
		# TODO: this should be run in both cases to handle any other dependencies, but it fails on the SDL frameworks and there currently aren't any other deps...

		file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fixup.cmake
			CONTENT "
				include(BundleUtilities)
				fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:${NAME}>.app\" \"\" \"\")
			"
		)

		install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/fixup.cmake)
	endif()
endfunction()

function(blit_executable_int_flash NAME SOURCES)
	blit_executable(${NAME} ${SOURCES} ${ARGN})
endfunction()

function(blit_metadata TARGET FILE)
	if(NOT EXISTS ${FILE})
		set(FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})
	endif()

	# cause cmake to reconfigure whenever the asset list changes
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${FILE})

	# parse the metadata to variables
	execute_process(COMMAND ${PYTHON_EXECUTABLE} -m ttblit cmake --config ${FILE} --cmake ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)
	include(${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)

	if(APPLE)
		set_target_properties(${TARGET} PROPERTIES
			OUTPUT_NAME ${METADATA_TITLE}
			MACOSX_BUNDLE_BUNDLE_NAME ${METADATA_TITLE}
			MACOSX_BUNDLE_SHORT_VERSION_STRING ${METADATA_VERSION}
			MACOSX_BUNDLE_LONG_VERSION_STRING ${METADATA_VERSION}
			MACOSX_BUNDLE_COPYRIGHT "${METADATA_AUTHOR}"
			MACOSX_BUNDLE_ICON_FILE icon
		)

		set(ICON ${CMAKE_CURRENT_BINARY_DIR}/icon.icns)

		add_custom_command(
			OUTPUT ${ICON}
			COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${PYTHON_EXECUTABLE} -m ttblit metadata --force --config ${FILE} --icns ${ICON}
			DEPENDS ${METADATA_DEPENDS} ${FILE}
		)

		target_sources(${TARGET} PRIVATE ${ICON})
		set_source_files_properties(${ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	endif()

	file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/_metadata.cpp
		CONTENT "
const char *metadata_title = \"${METADATA_TITLE}\";
const char *metadata_author = \"${METADATA_AUTHOR}\";
const char *metadata_description = \"${METADATA_DESCRIPTION}\";
const char *metadata_version = \"${METADATA_VERSION}\";
		"
	)
	target_sources(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/_metadata.cpp)
endfunction()

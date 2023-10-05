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

function (find_sdl_lib lib_name header_name)
	string(TOUPPER ${lib_name} VAR_PREFIX)

	# new enough SDL_image/_net have cmake config support
	find_package(${lib_name} QUIET)
	if(TARGET ${lib_name}::${lib_name})
		set(${VAR_PREFIX}_LIBRARY ${lib_name}::${lib_name} PARENT_SCOPE)
		set(${VAR_PREFIX}_INCLUDE_DIR "" PARENT_SCOPE)

		if(WIN32)
			get_property(LIB_DLL TARGET ${lib_name}::${lib_name} PROPERTY IMPORTED_LOCATION)
			set(${VAR_PREFIX}_DLL ${LIB_DLL} PARENT_SCOPE)
		endif()

		return()
	endif()

	message("find_package(${lib_name}) failed, trying manual search...")

	find_path(${VAR_PREFIX}_INCLUDE_DIR ${header_name}
		HINTS ${SDL2_DIR} ${SDL2_DIR}/../../../
		PATH_SUFFIXES SDL2 include/SDL2 include
	)

	find_library(${VAR_PREFIX}_LIBRARY
		NAMES ${lib_name}
		HINTS ${SDL2_DIR} ${SDL2_DIR}/../../../
		PATH_SUFFIXES lib
	)

	if(NOT ${VAR_PREFIX}_INCLUDE_DIR OR NOT ${VAR_PREFIX}_LIBRARY)
		message(FATAL_ERROR "${lib_name} not found!")
	endif()

endfunction()

if(EMSCRIPTEN)
	target_compile_options(BlitHalSDL
		PRIVATE -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_NET=2
	)

	set(SDL2_LIBRARIES "-sUSE_SDL=2")
	set(SDL2_IMAGE_LIBRARY "-sUSE_SDL_IMAGE=2")
	set(SDL2_NET_LIBRARY "-sUSE_SDL_NET=2")

	set(EMSCRIPTEN_SHELL ${CMAKE_CURRENT_SOURCE_DIR}/emscripten-shell.html PARENT_SCOPE)
else()

	# fallback guess for SDL location on Windows
	if(WIN32 AND NOT SDL2_DIR AND 32BLIT_DIR)
		set(SDL2_DIR "${32BLIT_DIR}/vs/sdl")
	endif()

	if(NOT TARGET SDL2::SDL2 AND NOT TARGET SDL2::SDL2-static)
		find_package(SDL2 REQUIRED)
	endif()

	if(SDL2_FRAMEWORK_PATH)
		set(SDL2_FRAMEWORK_PATH ${SDL2_FRAMEWORK_PATH} PARENT_SCOPE)
	endif()

	# If SDL2 was built using CMake, the generated configuration files define SDL2::* targets instead of the SDL2_* variables
	if(TARGET SDL2::SDL2)
		if(NOT TARGET SDL2::SDL2main)
			add_library(SDL2::SDL2main IMPORTED INTERFACE) # 2.24.0 is missing this target for macOS
		endif()

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
	find_sdl_lib(SDL2_image SDL_image.h)
	find_sdl_lib(SDL2_net SDL_net.h)

	target_include_directories(BlitHalSDL
		PRIVATE	${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_NET_INCLUDE_DIR}
	)
endif()

target_link_libraries(BlitHalSDL PUBLIC BlitEngine ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARY} ${SDL2_NET_LIBRARY})

# copy SDL2 dlls to build/install dir for windows users
set(DLLS_TO_COPY)

if(SDL2_DLL)
	list(APPEND DLLS_TO_COPY ${SDL2_DLL})
endif()

if(SDL2_IMAGE_DLL)
	list(APPEND DLLS_TO_COPY ${SDL2_IMAGE_DLL})
endif()

if(SDL2_NET_DLL)
	list(APPEND DLLS_TO_COPY ${SDL2_NET_DLL})
endif()

set(DLLS_TO_COPY ${DLLS_TO_COPY} PARENT_SCOPE)

# copy once for install
install(FILES ${DLLS_TO_COPY} DESTINATION bin)

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

function(blit_executable NAME)
	message(STATUS "Processing ${NAME}")
	blit_executable_args(${ARGN})

	if(ANDROID)
		add_library(${NAME} SHARED ${SOURCES})
	else()
		add_executable(${NAME} MACOSX_BUNDLE ${SOURCES})
	endif()

	install(TARGETS ${NAME}
		LIBRARY DESTINATION lib
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

	# copy dlls to build dir for dev
	foreach(DLL ${DLLS_TO_COPY})
		add_custom_command(TARGET ${NAME} POST_BUILD
    		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${DLL}"
			$<TARGET_FILE_DIR:${NAME}>
			VERBATIM
		)
	endforeach()

	if(EMSCRIPTEN)
		set_target_properties(${NAME} PROPERTIES
			SUFFIX ".html"
			LINK_FLAGS "-sENVIRONMENT=web -sSDL2_IMAGE_FORMATS=['jpg'] --shell-file ${EMSCRIPTEN_SHELL} -s'EXPORTED_RUNTIME_METHODS=[\"addRunDependency\", \"removeRunDependency\"]' -sTOTAL_STACK=1MB"
			LINK_DEPENDS ${EMSCRIPTEN_SHELL}
		)

		target_link_libraries(${NAME} "-lidbfs.js") # include the persistent IndexedDB-based filesystem

		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.js ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.wasm
			DESTINATION bin
		)
	endif()

	if(APPLE)
		set(SEARCH_DIRS /usr/local/lib)

		# assuming all frameworks installed in the same place
		if(SDL2_FRAMEWORK_PATH)
			get_filename_component(FW_PARENT_DIR ${SDL2_FRAMEWORK_PATH} DIRECTORY)
			list(APPEND SEARCH_DIRS ${FW_PARENT_DIR})
		endif()

		file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fixup.cmake
			CONTENT "
				include(BundleUtilities)
				fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:${NAME}>.app\" \"\" \"${SEARCH_DIRS}\")
			"
		)

		install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/fixup.cmake)
	endif()
endfunction()

function(blit_metadata TARGET FILE)
	if(NOT EXISTS ${FILE})
		set(FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})
	endif()

	# cause cmake to reconfigure whenever the asset list changes
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${FILE})

	# parse the metadata to variables
	execute_process(
		COMMAND ${32BLIT_TOOLS_EXECUTABLE} cmake --config ${FILE} --cmake ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake
		RESULT_VARIABLE TOOL_RESULT
	)
	if(${TOOL_RESULT})
		message(FATAL_ERROR "Reading metadata config failed!\n")
	endif()

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
			COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${32BLIT_TOOLS_EXECUTABLE} metadata --force --config ${FILE} --icns ${ICON}
			DEPENDS ${METADATA_DEPENDS} ${FILE}
			VERBATIM
		)

		target_sources(${TARGET} PRIVATE ${ICON})
		set_source_files_properties(${ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	endif()

	string(CONFIGURE "
const char *metadata_title = \"\${METADATA_TITLE}\";
const char *metadata_author = \"\${METADATA_AUTHOR}\";
const char *metadata_description = \"\${METADATA_DESCRIPTION}\";
const char *metadata_version = \"\${METADATA_VERSION}\";
const char *metadata_url = \"\${METADATA_URL}\";
const char *metadata_category = \"\${METADATA_CATEGORY}\";"
		METADATA_CONTENT ESCAPE_QUOTES
	)

	file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_metadata.cpp CONTENT "${METADATA_CONTENT}")

	target_sources(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_metadata.cpp)
endfunction()

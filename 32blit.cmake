if (NOT DEFINED BLIT_ONCE)
	set(BLIT_ONCE TRUE)

	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_EXTENSIONS OFF)

	if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
		set(CMAKE_BUILD_TYPE "Release")
	endif()
	
	if(WIN32)
		add_definitions("-DWIN32")
	endif()

	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/32blit 32blit)

	find_package(PythonInterp 3 REQUIRED)

	# tool paths
	set(ASSET_PACKER ${CMAKE_CURRENT_LIST_DIR}/tools/asset-packer)
	set(SPRITE_BUILDER ${CMAKE_CURRENT_LIST_DIR}/tools/sprite-builder)

	function(pack_sprites FILENAME TYPE OUT_PATH)
		# TODO: this will break if someone passes the same name in different subdirs
		get_filename_component(BASE_NAME ${FILENAME} NAME)
		add_custom_command(
			OUTPUT ${BASE_NAME}.blit
			COMMAND ${PYTHON_EXECUTABLE} ${SPRITE_BUILDER} --out ${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.blit ${TYPE} ${FILENAME}
			DEPENDS ${FILENAME} ${SPRITE_BUILDER}
		)

		set(${OUT_PATH} ${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.blit PARENT_SCOPE)
	endfunction()

	function (blit_assets TARGET)
		set(ASSET_TYPE "")
		set(ASSET_FILES)

		foreach(ARG IN LISTS ARGN)
			# set asset type
			if(ARG STREQUAL "RAW" OR ARG STREQUAL "SPRITE_PACKED" OR ARG STREQUAL "SPRITE_RAW")
				set(ASSET_TYPE ${ARG})
				continue()
			elseif(ASSET_TYPE STREQUAL "")
				message(FATAL_ERROR "No asset type specified")
			endif()

			set(ASSET_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${ARG})

			if(ASSET_TYPE STREQUAL "SPRITE_PACKED")
				pack_sprites(${ASSET_PATH} packed ASSET_PATH)
			elseif(ASSET_TYPE STREQUAL "SPRITE_RAW")
				pack_sprites(${ASSET_PATH} raw ASSET_PATH)
			endif()

			list(APPEND ASSET_FILES ${ASSET_PATH})
		endforeach()

		set(PACKER_ARGS)
		set(PACKER_OUTPUTS assets.bin assets.cpp assets.hpp)

		if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			set(PACKER_ARGS --inline-data)
			set(PACKER_OUTPUTS assets.cpp assets.hpp)
		endif()

		add_custom_command(
			OUTPUT ${PACKER_OUTPUTS}
			COMMAND ${PYTHON_EXECUTABLE} ${ASSET_PACKER} --base-path ${CMAKE_CURRENT_SOURCE_DIR} --base-path ${CMAKE_CURRENT_BINARY_DIR} ${PACKER_ARGS} ${ASSET_FILES}
			DEPENDS ${ASSET_FILES} ${ASSET_PACKER}
		)

		target_sources(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/assets.cpp)
		target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

		if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			add_custom_command(
				COMMAND ${CMAKE_LINKER} -r -b binary -o assets.bin.o assets.bin
				COMMAND ${CMAKE_OBJCOPY} --rename-section .data=.rodata,alloc,load,readonly,data,contents assets.bin.o assets.bin.o
				OUTPUT assets.bin.o
				DEPENDS assets.bin
			)

			target_sources(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/assets.bin.o)
		endif()
	endfunction()

	if (${CMAKE_SYSTEM_NAME} STREQUAL Generic)
		set(FLASH_PORT "AUTO" CACHE STRING "Port to use for flash")

		# attempt to find the upload tool
		find_program(32BLIT_TOOL 32Blit PATHS
			${CMAKE_CURRENT_BINARY_DIR}/../build/tools/src/
		)

		if(NOT 32BLIT_TOOL)
			message(WARNING "32Blit tool not found. Looking for 32Blit.exe instead")
			find_program(32BLIT_TOOL 32Blit.exe PATHS
				${CMAKE_CURRENT_BINARY_DIR}/../build.mingw/tools/src/
			)
		endif()

		if(NOT 32BLIT_TOOL)
			message(WARNING "32Blit tool not found")
		endif()

		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/32blit-stm32 32blit-stm32)
	else()
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/32blit-sdl 32blit-sdl)
	endif()
endif ()

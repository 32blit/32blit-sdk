if (NOT DEFINED BLIT_ONCE)
	set(BLIT_ONCE TRUE)

	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_EXTENSIONS OFF)

	if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
		set(CMAKE_BUILD_TYPE "Release")
	endif()

	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/32blit 32blit)

	function(blit_asset NAME ASSET)
		add_custom_command(
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ASSET}.o
			COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${CMAKE_LINKER} -r -b binary -o ${CMAKE_CURRENT_BINARY_DIR}/${ASSET}.o ${ASSET}
			COMMAND ${CMAKE_OBJCOPY} --rename-section .data=.rodata,alloc,load,readonly,data,contents ${CMAKE_CURRENT_BINARY_DIR}/${ASSET}.o ${CMAKE_CURRENT_BINARY_DIR}/${ASSET}.o
			DEPENDS ${ASSET}
		)
		# Add object as a library
		add_library(asset_${ASSET}
			STATIC
			${ASSET}.o)
		# Tell CMake this object file has already been built by custom command
		SET_SOURCE_FILES_PROPERTIES(
			${ASSET}.o
			PROPERTIES
			EXTERNAL_OBJECT true
			GENERATED true
		)
		# Avoid language warning
		SET_TARGET_PROPERTIES(
			asset_${ASSET}
			PROPERTIES
			LINKER_LANGUAGE C
		)
		# Link our asset to our output
		target_link_libraries(${NAME} asset_${ASSET})
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

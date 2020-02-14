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

	function(blit_asset NAME ASSET)
		string(REPLACE ".." "__" ASSET_OUT ${ASSET})
		set(ASSET_OUT ${CMAKE_CURRENT_BINARY_DIR}/${ASSET_OUT}.o)

		get_filename_component(OUT_DIR ${ASSET_OUT} DIRECTORY)

		add_custom_command(
			COMMAND mkdir -p ${OUT_DIR}
			COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${CMAKE_LINKER} -r -b binary -o ${ASSET_OUT} ${ASSET}
			COMMAND ${CMAKE_OBJCOPY} --rename-section .data=.rodata,alloc,load,readonly,data,contents ${ASSET_OUT} ${ASSET_OUT}
			OUTPUT ${ASSET_OUT}
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ASSET}
		)

		target_sources(${NAME} PRIVATE ${ASSET_OUT})
	endfunction()

	function (blit_assets TARGET)
		foreach(ARG IN LISTS ARGN)
			blit_asset(${TARGET} ${ARG})
		endforeach()
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

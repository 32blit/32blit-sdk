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

	function (blit_assets_yaml TARGET FILE)
		# cause cmake to reconfigure whenever the asset list changes
		set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})

		# get the inputs/outputs for the asset tool (at configure time)
		execute_process(COMMAND ${PYTHON_EXECUTABLE} -m ttblit --debug cmake --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --output ${CMAKE_CURRENT_BINARY_DIR} --cmake ${CMAKE_CURRENT_BINARY_DIR}/assets.cmake)
		include(${CMAKE_CURRENT_BINARY_DIR}/assets.cmake)

		# do asset packing (at build time)
		add_custom_command(
			OUTPUT ${ASSET_OUTPUTS}
			COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${PYTHON_EXECUTABLE} -m ttblit --debug  pack --force --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --output ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS ${ASSET_DEPENDS} ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
		)

		# add the outputs as dependencies of the project (also compile any cpp files)
		target_sources(${TARGET} PRIVATE ${ASSET_OUTPUTS})
		target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
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

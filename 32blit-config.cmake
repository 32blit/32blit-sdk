if (NOT DEFINED BLIT_ONCE)
	set(BLIT_ONCE TRUE)

	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_EXTENSIONS OFF)
	set(BLIT_MINIMUM_TOOLS_VERSION "0.7.3")

	find_package(PythonInterp 3.6 REQUIRED)

	# make sure that the tools are installed
	execute_process(COMMAND ${PYTHON_EXECUTABLE} -m ttblit version RESULT_VARIABLE VERSION_STATUS OUTPUT_VARIABLE TOOLS_VERSION ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

	if(${VERSION_STATUS})
		# check for the executable if the module isn't found
		find_program(32BLIT_TOOLS_EXECUTABLE 32blit)
		if(32BLIT_TOOLS_EXECUTABLE)
			# get the version
			execute_process(COMMAND ${32BLIT_TOOLS_EXECUTABLE} version RESULT_VARIABLE VERSION_STATUS OUTPUT_VARIABLE TOOLS_VERSION ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
		endif()
	else()
		set(32BLIT_TOOLS_EXECUTABLE ${PYTHON_EXECUTABLE} -m ttblit)
	endif()

	# get just the python command to output to the user
	get_filename_component(PYTHON_USER_EXECUTABLE "${PYTHON_EXECUTABLE}" NAME)

	if(${VERSION_STATUS})
		message(FATAL_ERROR "32Blit tools not found!\nInstall with: ${PYTHON_USER_EXECUTABLE} -m pip install 32blit\n")
	endif()

	if("${TOOLS_VERSION}" VERSION_LESS "${BLIT_MINIMUM_TOOLS_VERSION}")
		message(FATAL_ERROR "32Blit tools out of date!\nYou have ${TOOLS_VERSION}, we need >= ${BLIT_MINIMUM_TOOLS_VERSION}\nUpdate with: ${PYTHON_USER_EXECUTABLE} -m pip install --upgrade 32blit\n")
	endif()

	if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
		set(CMAKE_BUILD_TYPE "Release")
	endif()

	if(WIN32)
		add_definitions("-DWIN32")
	endif()

	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rd-party 3rd-party)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/32blit 32blit)

	function (blit_assets_yaml TARGET FILE)
		# cause cmake to reconfigure whenever the asset list changes
		set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})

		# get the inputs/outputs for the asset tool (at configure time)
		execute_process(
		    COMMAND ${32BLIT_TOOLS_EXECUTABLE} --debug cmake --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --output ${CMAKE_CURRENT_BINARY_DIR} --cmake ${CMAKE_CURRENT_BINARY_DIR}/assets.cmake
            RESULT_VARIABLE TOOL_RESULT
		)
		if(${TOOL_RESULT})
		    message(FATAL_ERROR "Reading asset config failed!\n")
		endif()

		include(${CMAKE_CURRENT_BINARY_DIR}/assets.cmake)

		# do asset packing (at build time)
		add_custom_command(
			OUTPUT ${ASSET_OUTPUTS}
			COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${32BLIT_TOOLS_EXECUTABLE} --debug  pack --force --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --output ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS ${ASSET_DEPENDS} ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
			VERBATIM
		)

		# add the outputs as dependencies of the project (also compile any cpp files)
		target_sources(${TARGET} PRIVATE ${ASSET_OUTPUTS})
		target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
	endfunction()

	# common helpers
	function(blit_executable_args)
		set(SOURCES)

		if(DEFINED BLIT_EXECUTABLE_INTERNAL_FLASH)
			set(INTERNAL_FLASH ${BLIT_EXECUTABLE_INTERNAL_FLASH})
		else()
			set(INTERNAL_FLASH FALSE)
		endif()

		foreach(arg IN LISTS ARGN)
			if(arg STREQUAL "INTERNAL_FLASH")
				set(${arg} TRUE)
			else()
				list(APPEND SOURCES ${arg})
			endif()
		endforeach()

		set(SOURCES ${SOURCES} PARENT_SCOPE)
		set(INTERNAL_FLASH ${INTERNAL_FLASH} PARENT_SCOPE)
	endfunction()

	if (32BLIT_HW)
		set(FLASH_PORT "AUTO" CACHE STRING "Port to use for flash")

		include(${CMAKE_CURRENT_LIST_DIR}/32blit-stm32/executable.cmake)
	elseif(PICO_SDK_PATH)
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/32blit-pico 32blit-pico)
	else()
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/32blit-sdl 32blit-sdl)
	endif()
endif ()

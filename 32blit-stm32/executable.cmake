enable_language(ASM)

set(MCU_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/${MCU_LINKER_SCRIPT}")
set(HAL_DIR ${CMAKE_CURRENT_LIST_DIR})

set(USER_STARTUP ${CMAKE_CURRENT_LIST_DIR}/startup_user.S ${CMAKE_CURRENT_LIST_DIR}/startup_user.cpp ${CMAKE_CURRENT_LIST_DIR}/Src/api.cpp)

function(blit_executable_common NAME)
	target_link_libraries(${NAME} BlitEngine)
	set_property(TARGET ${NAME} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-Map=${NAME}.map,--cref")
	add_custom_command(TARGET ${NAME} POST_BUILD
		COMMENT "Building ${NAME}.bin"
		COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:${NAME}> ${NAME}.bin
		COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${NAME}>
		VERBATIM
	)
endfunction()

function(blit_executable NAME)
	message(STATUS "Processing ${NAME}")
	blit_executable_args(${ARGN})

	if(INTERNAL_FLASH)
		# make sure the HAL is built (external projects)
		if(NOT TARGET BlitHalSTM32)
			add_subdirectory(${HAL_DIR} 32blit-stm32)
		endif()

		blit_executable_int_flash(${NAME} ${SOURCES})
		return()
	endif()

	add_executable(${NAME} ${USER_STARTUP} ${SOURCES})

	set(BLIT_FILENAME $<TARGET_FILE_BASE_NAME:${NAME}>.blit)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BLIT_FILENAME}
		DESTINATION bin
	)

	set_target_properties(${NAME} PROPERTIES
		LINK_FLAGS "-specs=nano.specs -u _printf_float -fPIC -T ${MCU_LINKER_SCRIPT} ${MCU_LINKER_FLAGS_EXT} -Wl,--emit-relocs"
	)
	set_target_properties(${NAME} PROPERTIES LINK_DEPENDS ${MCU_LINKER_SCRIPT} SUFFIX ".elf")

	blit_executable_common(${NAME})
	target_link_libraries(${NAME} ${PIC_STDLIBS})

	add_custom_command(TARGET ${NAME} POST_BUILD
		COMMENT "Building ${NAME}.blit"
		COMMAND ${32BLIT_TOOLS_EXECUTABLE} relocs --elf-file $<TARGET_FILE:${NAME}> --bin-file ${NAME}.bin --output ${BLIT_FILENAME}
		VERBATIM
	)

	add_custom_target(${NAME}.flash DEPENDS ${NAME} COMMAND ${32BLIT_TOOLS_EXECUTABLE} install --port=${FLASH_PORT} --launch ${CMAKE_CURRENT_BINARY_DIR}/${BLIT_FILENAME})
endfunction()

function(blit_metadata TARGET FILE)
	if(NOT IS_ABSOLUTE ${FILE})
		set(FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})
	endif()

	# cause cmake to reconfigure whenever the asset list changes
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${FILE})

	# get the inputs/outputs for the asset tool (at configure time)
	execute_process(
		COMMAND ${32BLIT_TOOLS_EXECUTABLE} cmake --config ${FILE} --cmake ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake
		RESULT_VARIABLE TOOL_RESULT
	)
	if(${TOOL_RESULT})
		message(FATAL_ERROR "Reading metadata config failed!\n")
	endif()

	include(${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)

	set(BLIT_FILENAME $<TARGET_FILE_BASE_NAME:${TARGET}>.blit)

	add_custom_command(
		TARGET ${TARGET} POST_BUILD
		COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${32BLIT_TOOLS_EXECUTABLE} metadata --config ${FILE} --file ${CMAKE_CURRENT_BINARY_DIR}/${BLIT_FILENAME}
		VERBATIM
	)

	# force relink on change so that the post-build commands are rerun
	set_property(TARGET ${TARGET} APPEND PROPERTY LINK_DEPENDS ${FILE} ${METADATA_DEPENDS})
endfunction()

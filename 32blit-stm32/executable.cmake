set(MCU_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/${MCU_LINKER_SCRIPT}")

set(USER_STARTUP ${CMAKE_CURRENT_LIST_DIR}/startup_user.s ${CMAKE_CURRENT_LIST_DIR}/startup_user.cpp)

function(blit_executable_common NAME)
	target_link_libraries(${NAME} BlitEngine)
	set_property(TARGET ${NAME} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-Map=${NAME}.map,--cref")
	add_custom_command(TARGET ${NAME} POST_BUILD
		COMMENT "Building ${NAME}.bin"
		COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${NAME}> ${NAME}.hex
		COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:${NAME}> ${NAME}.bin
		COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${NAME}>
		COMMAND ${CMAKE_READELF} -S $<TARGET_FILE:${NAME}>
	)
endfunction()

function(blit_executable NAME SOURCES)
	set_source_files_properties(${USER_STARTUP} PROPERTIES LANGUAGE CXX)
	add_executable(${NAME} ${USER_STARTUP} ${SOURCES} ${ARGN})

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.bin
		DESTINATION bin
	)

	set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-specs=nano.specs -u _printf_float -T ${MCU_LINKER_SCRIPT} ${MCU_LINKER_FLAGS_EXT}")
	set_target_properties(${NAME} PROPERTIES LINK_DEPENDS ${MCU_LINKER_SCRIPT} SUFFIX ".elf")

	blit_executable_common(${NAME})

	add_custom_target(${NAME}.flash DEPENDS ${NAME} COMMAND ${PYTHON_EXECUTABLE} -m ttblit flash --port=${FLASH_PORT} flash --file=${CMAKE_CURRENT_BINARY_DIR}/${NAME}.bin)
endfunction()

function(blit_metadata TARGET FILE)
	# cause cmake to reconfigure whenever the asset list changes
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})

	# get the inputs/outputs for the asset tool (at configure time)
	execute_process(COMMAND ${PYTHON_EXECUTABLE} -m ttblit cmake --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --cmake ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)
	include(${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)

	add_custom_command(
		TARGET ${TARGET} POST_BUILD
		COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${PYTHON_EXECUTABLE} -m ttblit metadata --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --file ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.bin
	)

	# force relink on change so that the post-build commands are rerun
	set_property(TARGET ${TARGET} APPEND PROPERTY LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} ${METADATA_DEPENDS})
endfunction()

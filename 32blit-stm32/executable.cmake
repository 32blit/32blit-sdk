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

	if(32BLIT_TOOL)
		add_custom_target(${NAME}.flash DEPENDS ${NAME} COMMAND ${32BLIT_TOOL} PROG ${FLASH_PORT} ${NAME}.bin)
	endif()
endfunction()

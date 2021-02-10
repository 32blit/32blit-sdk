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
	message(STATUS "Processing ${NAME}")
	set_source_files_properties(${USER_STARTUP} PROPERTIES LANGUAGE CXX)
	add_executable(${NAME} ${USER_STARTUP} ${SOURCES} ${ARGN})

	# Ideally we want the .blit filename to match the .elf, but TARGET_FILE_BASE_NAME isn't always available
	# (This only affects the firmware updater as it's the only thing setting a custom OUTPUT_NAME)
	if(${CMAKE_VERSION} VERSION_LESS "3.15.0")
		set(BLIT_FILENAME ${NAME}.blit)
	else()
		set(BLIT_FILENAME $<TARGET_FILE_BASE_NAME:${NAME}>.blit)
	endif()

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
		COMMAND ${PYTHON_EXECUTABLE} -m ttblit relocs --elf-file $<TARGET_FILE:${NAME}> --bin-file ${NAME}.bin --output ${BLIT_FILENAME}
	)

	add_custom_target(${NAME}.flash DEPENDS ${NAME} COMMAND ${PYTHON_EXECUTABLE} -m ttblit flash flash --port=${FLASH_PORT} --file=${CMAKE_CURRENT_BINARY_DIR}/${BLIT_FILENAME})
endfunction()

function(blit_metadata TARGET FILE)
	if(NOT IS_ABSOLUTE ${FILE})
		set(FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})
	endif()

	# cause cmake to reconfigure whenever the asset list changes
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${FILE})

	# get the inputs/outputs for the asset tool (at configure time)
	execute_process(COMMAND ${PYTHON_EXECUTABLE} -m ttblit cmake --config ${FILE} --cmake ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)
	include(${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)

	if(${CMAKE_VERSION} VERSION_LESS "3.15.0")
		set(BLIT_FILENAME ${TARGET}.blit)
	else()
		set(BLIT_FILENAME $<TARGET_FILE_BASE_NAME:${TARGET}>.blit)
	endif()

	add_custom_command(
		TARGET ${TARGET} POST_BUILD
		COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${PYTHON_EXECUTABLE} -m ttblit metadata --config ${FILE} --file ${CMAKE_CURRENT_BINARY_DIR}/${BLIT_FILENAME}
	)

	# force relink on change so that the post-build commands are rerun
	set_property(TARGET ${TARGET} APPEND PROPERTY LINK_DEPENDS ${FILE} ${METADATA_DEPENDS})
endfunction()

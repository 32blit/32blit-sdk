set(CMAKE_C_STANDARD 11)

# Initialise the Pico SDK
pico_sdk_init()

set(32BLIT_PICO 1 PARENT_SCOPE)

add_library(BlitHalPico INTERFACE)
target_sources(BlitHalPico INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/st7789.cpp
)
target_link_libraries(BlitHalPico INTERFACE hardware_dma hardware_pwm hardware_spi pico_stdlib)

function(blit_executable_common NAME)
	target_link_libraries(${NAME} BlitEngine)

endfunction()

function(blit_executable_int_flash NAME SOURCES)
    add_executable(${NAME} ${SOURCES} ${ARGN})
    target_link_libraries(${NAME} BlitHalPico BlitEngine)

    pico_enable_stdio_uart(${NAME} 1)
    pico_enable_stdio_usb(${NAME} 0)

    pico_add_extra_outputs(${NAME})

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
		DESTINATION bin
	)
endfunction()

function(blit_executable NAME SOURCES)
	blit_executable_int_flash(${NAME} ${SOURCES} ${ARGN})
endfunction()

function(blit_metadata TARGET FILE)
	# cause cmake to reconfigure whenever the asset list changes
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})

	# get the inputs/outputs for the asset tool (at configure time)
	execute_process(COMMAND ${PYTHON_EXECUTABLE} -m ttblit cmake --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --cmake ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)
	include(${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)

	pico_set_program_name(${TARGET} "${METADATA_TITLE}")
	pico_set_program_description(${TARGET} "${METADATA_DESCRIPTION}")
	pico_set_program_version(${TARGET} "${METADATA_VERSION}")
	pico_set_program_url(${TARGET} "${METADATA_URL}")
endfunction()
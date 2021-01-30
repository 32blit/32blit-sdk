set(CMAKE_C_STANDARD 11)

include(${PICO_SDK_PATH}/../pimoroni-pico/drivers/st7789/st7789.cmake)

# Initialise the Pico SDK
pico_sdk_init()

add_library(BlitHalPico INTERFACE)
target_sources(BlitHalPico INTERFACE ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
target_link_libraries(BlitHalPico INTERFACE pico_stdlib st7789) # need the include paths

#pico_set_program_name(test-project "test-project")
#pico_set_program_version(test-project "0.1")

#shouldn't really need this
target_include_directories(BlitHalPico INTERFACE ${PICO_SDK_PATH}/../pimoroni-pico/drivers/st7789/)

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
	#set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})

	# get the inputs/outputs for the asset tool (at configure time)
	#execute_process(COMMAND ${PYTHON_EXECUTABLE} -m ttblit cmake --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --cmake ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)
	#include(${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake)

	#add_custom_command(
	#	TARGET ${TARGET} POST_BUILD
	#	COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${PYTHON_EXECUTABLE} -m ttblit metadata --config ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} --file ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.reloc.bin
	#)

	# force relink on change so that the post-build commands are rerun
	#set_property(TARGET ${TARGET} APPEND PROPERTY LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} ${METADATA_DEPENDS})
endfunction()

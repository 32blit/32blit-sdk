list(APPEND SOURCES
	Src/main.c
	Src/stm32h7xx_it.c
	Src/stm32h7xx_hal_msp.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hrtim.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smbus.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dsi.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c
	Middlewares/Third_Party/FatFs/src/diskio.c
	Middlewares/Third_Party/FatFs/src/ff.c
	Middlewares/Third_Party/FatFs/src/ff_gen_drv.c
	Middlewares/Third_Party/FatFs/src/option/syscall.c
	Middlewares/Third_Party/FatFs/src/option/ccsbcs.c
	Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
	Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
	Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
	Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c
	Src/system_stm32h7xx.c
	Src/user_diskio.c
	Src/fatfs.c
	Src/fatfs_sd.c
	Src/usb_otg.c
	Src/usb_device.c
	Src/usbd_conf.c
	Src/usbd_desc.c
	Src/usbd_cdc_if.c
	Src/display.cpp
	Src/gpio.cpp
	Src/sound.cpp
	Src/adc.c
	Src/debug.c
	Src/dma.c
	Src/hrtim.c
	Src/i2c.c
	Src/i2c-msa301.c
	Src/i2c-bq24295.c
	Src/quadspi.c
	Src/spi.c
	Src/spi-st7272a.c
	Src/sys.c
	Src/tim.c
	Src/rng.c
	Src/32blit.c
	Src/CDCLogging.c
	startup_stm32h750xx.s
	Src/CDCDataStream.cpp 
	Src/CDCCommandStream.cpp 
	Src/CDCCommandHandler.cpp 
	Src/CDCResetHandler.cpp
	Src/CDCInfoHandler.cpp 
)

set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE CXX )

add_library(BlitHalSTM32 OBJECT ${SOURCES})

set(INCLUDE_DIRS 
	${CMAKE_CURRENT_SOURCE_DIR}/Inc
	${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
	${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
	${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FatFs/src
	${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
	${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
	${CMAKE_CURRENT_SOURCE_DIR}/../32blit
)
set(HAL_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)

set(DEFINITIONS
	-DUSE_HAL_DRIVER
	-DUSE_USB_HS
	-DSTM32H750xx
	-DCDC_FIFO_BUFFERS=${CDC_FIFO_BUFFERS}
)
set(HAL_DEFINITIONS ${DEFINITIONS} PARENT_SCOPE)

set_source_files_properties(Middlewares/Third_Party/FatFs/src/ff.c PROPERTIES COMPILE_FLAGS "-Wno-misleading-indentation")

target_include_directories(BlitHalSTM32
	PRIVATE
		${INCLUDE_DIRS}
)

target_compile_definitions(BlitHalSTM32
	PRIVATE
		${DEFINITIONS}
		-DAPPLICATION_VTOR=${APPLICATION_VTOR}
		-DEXTERNAL_LOAD_ADDRESS=${EXTERNAL_LOAD_ADDRESS}
		-DINITIALISE_QSPI=${INITIALISE_QSPI}
)

target_compile_options(BlitHalSTM32
	PRIVATE
		-fpermissive
)
target_compile_options(BlitHalSTM32 PUBLIC "$<$<CONFIG:RELEASE>:-Os>")

# 2nd build for external flash
add_library(BlitHalSTM32Ext OBJECT ${SOURCES})

target_include_directories(BlitHalSTM32Ext
	PRIVATE
		${INCLUDE_DIRS}
)

target_compile_definitions(BlitHalSTM32Ext
	PRIVATE
		${DEFINITIONS}
		-DAPPLICATION_VTOR=${APPLICATION_VTOR_EXT}
		-DEXTERNAL_LOAD_ADDRESS=${EXTERNAL_LOAD_ADDRESS_EXT}
		-DINITIALISE_QSPI=${INITIALISE_QSPI_EXT}
)

target_compile_options(BlitHalSTM32Ext
	PRIVATE
		-fpermissive
)

set(MCU_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/${MCU_LINKER_SCRIPT}")
set(MCU_LINKER_SCRIPT "${MCU_LINKER_SCRIPT}" PARENT_SCOPE)
set(MCU_LINKER_SCRIPT_EXT "${CMAKE_CURRENT_LIST_DIR}/${MCU_LINKER_SCRIPT_EXT}" PARENT_SCOPE)

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
	add_executable(${NAME} $<TARGET_OBJECTS:BlitHalSTM32Ext> ${SOURCES} ${ARGN})

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.bin
		DESTINATION bin
	)

	set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-specs=nano.specs -T ${MCU_LINKER_SCRIPT_EXT}")
	set_target_properties(${NAME} PROPERTIES LINK_DEPENDS ${MCU_LINKER_SCRIPT_EXT} SUFFIX ".elf")

	blit_executable_common(${NAME})

	if(32BLIT_TOOL)
		add_custom_target(${NAME}.flash DEPENDS ${NAME} COMMAND ${32BLIT_TOOL} PROG ${FLASH_PORT} ${NAME}.bin)
	endif()
endfunction()

function(blit_executable_int_flash NAME SOURCES)
	add_executable(${NAME} $<TARGET_OBJECTS:BlitHalSTM32> ${SOURCES} ${ARGN})

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.bin ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.dfu
		DESTINATION bin
	)

	set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-specs=nano.specs -T ${MCU_LINKER_SCRIPT}")
	set_target_properties(${NAME} PROPERTIES LINK_DEPENDS ${MCU_LINKER_SCRIPT} SUFFIX ".elf")

	blit_executable_common(${NAME})

	target_include_directories(${NAME} PRIVATE ${HAL_INCLUDE_DIRS})
	target_compile_definitions(${NAME} PRIVATE ${HAL_DEFINITIONS})

	add_custom_command(TARGET ${NAME} POST_BUILD
		COMMENT "Building ${NAME}.dfu"
		COMMAND ${CMAKE_DFU} build --force --out ${NAME}.dfu ${NAME}.bin
	)
endfunction()

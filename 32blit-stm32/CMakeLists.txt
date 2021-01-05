list(APPEND SOURCES
	startup_stm32h750xx.s
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
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.c
	Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.c
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
	Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc.c
	Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_bot.c
	Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_data.c
	Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_core.c
	Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_ctlreq.c
	Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_ioreq.c
	Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_pipes.c
	Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Src/usbh_cdc.c
	Src/usbd_msc_scsi.c
	Src/system_stm32h7xx.c
	Src/user_diskio.c
	Src/fatfs.c
	Src/fatfs_sd.c
	Src/usb_otg.c
	Src/usb_device.c
	Src/usb_host.c
	Src/usbd_conf.c
	Src/usbd_desc.c
	Src/usbd_cdc_if.c
	Src/usbh_conf.c
	Src/usbd_storage_if.c
	Src/display.cpp
	Src/gpio.cpp
	Src/jpeg.c
	Src/multiplayer.cpp
	Src/sound.cpp
	Src/file.cpp
	Src/jpeg.cpp
	Src/adc.c
	Src/debug.c
	Src/dma.c
	Src/hrtim.c
	Src/i2c.c
	Src/i2c-msa301.c
	Src/i2c-lis3dh.c
	Src/i2c-bq24295.c
	Src/quadspi.c
	Src/spi.c
	Src/spi-st7272a.c
	Src/sys.c
	Src/tim.c
	Src/rng.c
	Src/32blit/32blit_battery.cpp
	Src/32blit/32blit_i2c.cpp
	Src/SystemMenu/firmware_menu.cpp
	Src/SystemMenu/system_menu_controller.cpp
	Src/SystemMenu/battery_menu.cpp
	Src/SystemMenu/connectivity_menu.cpp
	Src/SystemMenu/about_menu.cpp
	Src/32blit.cpp
	Src/CDCDataStream.cpp
	Src/CDCCommandStream.cpp
	Src/CDCCommandHandler.cpp
	Src/CDCResetHandler.cpp
	Src/CDCInfoHandler.cpp
	Src/USBManager.cpp
	Utilities/JPEG/jpeg_utils.c
)

set_source_files_properties(
	startup_stm32h750xx.s

	Src/main.c
  	Src/usbd_msc_scsi.c
	Src/usb_device.c
	Src/usbd_desc.c
	Src/usbd_cdc_if.c
	Src/usbd_storage_if.c

	PROPERTIES LANGUAGE CXX
)

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
	${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc
	${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Host_Library/Core/Inc
	${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc
	${CMAKE_CURRENT_SOURCE_DIR}/Utilities
	${CMAKE_CURRENT_SOURCE_DIR}/../32blit
	${CMAKE_CURRENT_SOURCE_DIR}/../launcher-shared
)
set(HAL_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)

set(DEFINITIONS
	-DUSE_HAL_DRIVER
	-DUSE_USB_HS
	-DSTM32H750xx
	-DMSC_MEDIA_PACKET=4096
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
    -DCDC_FIFO_BUFFERS=${CDC_FIFO_BUFFERS}
)

target_compile_options(BlitHalSTM32 PUBLIC "$<$<CONFIG:RELEASE>:-Os>")
target_compile_options(BlitHalSTM32 PUBLIC "$<$<CONFIG:Debug>:-finline-functions-called-once>") # need at least some inlining, otherwise build is too big
set_target_properties(BlitHalSTM32 PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
set_target_properties(BlitHalSTM32 PROPERTIES POSITION_INDEPENDENT_CODE OFF)

function(blit_executable_int_flash NAME SOURCES)
	add_executable(${NAME} $<TARGET_OBJECTS:BlitHalSTM32> ${SOURCES} ${ARGN})

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.bin ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.dfu
		DESTINATION bin
	)

	set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-specs=nano.specs -T ${MCU_LINKER_SCRIPT} ${MCU_LINKER_FLAGS_INT}")
	set_target_properties(${NAME} PROPERTIES LINK_DEPENDS ${MCU_LINKER_SCRIPT} SUFFIX ".elf")
	set_target_properties(${NAME} PROPERTIES POSITION_INDEPENDENT_CODE OFF)

	blit_executable_common(${NAME})
	target_link_libraries(${NAME} ${NOPIC_STDLIBS})

	target_include_directories(${NAME} PRIVATE ${HAL_INCLUDE_DIRS})
	target_compile_definitions(${NAME} PRIVATE ${HAL_DEFINITIONS})
	target_compile_options(${NAME} PUBLIC "$<$<CONFIG:RELEASE>:-Os>") # always optimise for size if targetting internal flash

	add_custom_command(TARGET ${NAME} POST_BUILD
		COMMENT "Building ${NAME}.dfu"
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_DFU} build --force --out ${NAME}.dfu ${NAME}.bin
	)
endfunction()

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
	Src/gpio.c
	Src/adc.c
	Src/dac.c
	Src/debug.c
	Src/dma.c
	Src/hrtim.c
	Src/i2c.c
	Src/i2c-msa301.c
	Src/i2c-bq24295.c
	Src/ltdc.c
	Src/quadspi.c
	Src/spi.c
	Src/spi-st7272a.c
	Src/sys.c
	Src/tim.c
	Src/rng.c
	Src/32blit.c
	Src/CDCLogging.c
	startup_stm32h750xx.s
)

set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE CXX )

add_library(BlitHalSTM32 OBJECT ${SOURCES})


target_include_directories(BlitHalSTM32
	PRIVATE
		Inc
		Drivers/STM32H7xx_HAL_Driver/Inc
		Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
		Drivers/CMSIS/Device/ST/STM32H7xx/Include
		Drivers/CMSIS/Include
		Drivers/CMSIS/Include
		Middlewares/Third_Party/FatFs/src
		Middlewares/ST/STM32_USB_Device_Library/Core/Inc
		Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
		../32blit
)

target_compile_definitions(BlitHalSTM32
	PRIVATE
		-DUSE_HAL_DRIVER
		-DUSE_USB_HS
		-DSTM32H750xx
)

set(MCU_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/${MCU_LINKER_SCRIPT}" PARENT_SCOPE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -specs=nano.specs -T ${CMAKE_CURRENT_LIST_DIR}/${MCU_LINKER_SCRIPT}" PARENT_SCOPE)

function(blit_executable NAME SOURCES)
	add_executable(${NAME}.elf $<TARGET_OBJECTS:BlitHalSTM32> ${SOURCES} ${ARGN})
  	target_link_libraries(${NAME}.elf BlitEngine)
	set_target_properties(${NAME}.elf PROPERTIES LINK_DEPENDS ${MCU_LINKER_SCRIPT})
	set_property(TARGET ${NAME}.elf APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-Map=${NAME}.map,--cref")
  	add_custom_command(TARGET ${NAME}.elf POST_BUILD
		COMMENT "Building ${NAME}.bin\nBuilding ${NAME}.dfu"
		COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${NAME}.elf> ${NAME}.hex
		COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:${NAME}.elf> ${NAME}.bin
		COMMAND ${CMAKE_DFU} build --force --out ${NAME}.dfu ${NAME}.bin
		COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${NAME}.elf>
	)
endfunction()

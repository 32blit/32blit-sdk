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
	Src/system_stm32h7xx.c
	Src/user_diskio.c
	Src/fatfs.c
	Src/fatfs_sd.c
	Middlewares/Third_Party/FatFs/src/diskio.c
	Middlewares/Third_Party/FatFs/src/ff.c
	Middlewares/Third_Party/FatFs/src/ff_gen_drv.c
	Middlewares/Third_Party/FatFs/src/option/syscall.c
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
	Src/usb_otg.c
	Src/32blit.c
	startup_stm32h750xx.s
)

set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE CXX )

add_library(BlitHalSTM32 STATIC ${SOURCES})


target_include_directories(BlitHalSTM32
	PRIVATE
		Inc
		Drivers/STM32H7xx_HAL_Driver/Inc
		Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
		Drivers/CMSIS/Device/ST/STM32H7xx/Include
		Drivers/CMSIS/Include
		Drivers/CMSIS/Include
		Middlewares/Third_Party/FatFs/src
)

target_link_libraries(BlitHalSTM32 BlitEngine)

target_compile_definitions(BlitHalSTM32
	PUBLIC
		-DUSE_HAL_DRIVER
		-DUSE_USB_HS
		-DSTM32H750xx
)

set(MCU_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/${MCU_LINKER_SCRIPT}" PARENT_SCOPE)
set(CMAKE_EXE_LINKER_FLAGS "-specs nosys.specs -Wl,--gc-sections -T ${CMAKE_CURRENT_LIST_DIR}/${MCU_LINKER_SCRIPT}" PARENT_SCOPE)

function(blit_executable NAME SOURCES)
	add_executable(${NAME}.elf ${SOURCES} ${ARGN})
  	target_link_libraries(${NAME}.elf BlitHalSTM32 BlitEngine)
	set_target_properties(${NAME}.elf PROPERTIES LINK_DEPENDS ${MCU_LINKER_SCRIPT})
  	add_custom_command(TARGET ${NAME}.elf POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${NAME}.elf> ${NAME}.hex
		COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${NAME}.elf> ${NAME}.bin
		COMMENT "Building ${NAME}.hex \nBuilding ${NAME}.bin"
	)
endfunction()

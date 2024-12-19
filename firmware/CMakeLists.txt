cmake_minimum_required(VERSION 3.9...3.26)
project (firmware)
find_package (32BLIT CONFIG REQUIRED PATHS ..)

blit_executable(firmware INTERNAL_FLASH firmware.cpp)
target_link_libraries(firmware LauncherShared)

target_compile_definitions(firmware
  PRIVATE
 	-DCDC_FIFO_BUFFERS=${CDC_FIFO_BUFFERS}
)

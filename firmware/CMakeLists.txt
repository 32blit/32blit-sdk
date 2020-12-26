cmake_minimum_required(VERSION 3.9)
project (firmware)
include (../32blit.cmake)

blit_executable_int_flash(firmware firmware.cpp)
target_link_libraries(firmware LauncherShared)

target_compile_definitions(firmware
  PRIVATE
 	-DCDC_FIFO_BUFFERS=${CDC_FIFO_BUFFERS}
)

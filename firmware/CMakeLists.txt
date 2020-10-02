cmake_minimum_required(VERSION 3.8)
project (firmware)
include (../32blit.cmake)

blit_executable_int_flash(firmware firmware.cpp metadata.cpp)


target_compile_definitions(firmware
  PRIVATE
 	-DCDC_FIFO_BUFFERS=${CDC_FIFO_BUFFERS}
)

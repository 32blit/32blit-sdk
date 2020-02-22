cmake_minimum_required(VERSION 3.1)
project (flash-loader)
include (../../32blit.cmake)

blit_executable_int_flash(flash-loader flash-loader.cpp)

target_compile_definitions(flash-loader
	PRIVATE
  	-DCDC_FIFO_BUFFERS=${CDC_FIFO_BUFFERS}
)


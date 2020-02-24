cmake_minimum_required(VERSION 3.1)
project (firmware)
include (../32blit.cmake)

blit_executable_int_flash(firmware firmware.cpp)

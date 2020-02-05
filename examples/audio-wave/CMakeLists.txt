cmake_minimum_required(VERSION 2.8.9)
project (audio-wave)
include (../../32blit.cmake)
blit_executable_int_flash (audio-wave audio-wave.cpp)

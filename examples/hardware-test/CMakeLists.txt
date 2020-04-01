cmake_minimum_required(VERSION 3.8)
project (hardware-test)
include (../../32blit.cmake)
blit_executable (hardware-test hardware-test.cpp)

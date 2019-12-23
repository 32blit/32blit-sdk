cmake_minimum_required(VERSION 2.8.9)
project (hardware-test)
include (../../32blit.cmake)
blit_executable (hardware-test hardware-test.cpp)

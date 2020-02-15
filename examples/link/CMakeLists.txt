cmake_minimum_required(VERSION 3.1)
project (link)
include (../../32blit.cmake)
blit_executable (link link.cpp 3d/camera.cpp object.cpp renderer.cpp)

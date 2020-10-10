cmake_minimum_required(VERSION 3.8)
project (animated_gif)
include (../../32blit.cmake)
blit_executable (animated_gif animated_gif.cpp AnimatedGIF.cpp)

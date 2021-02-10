cmake_minimum_required(VERSION 3.9)
project (racer)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)
blit_executable (racer racer.cpp)

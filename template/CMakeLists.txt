cmake_minimum_required(VERSION 3.9)
project (game)
find_package (32BLIT CONFIG REQUIRED PATHS ..)
blit_executable (game game.cpp game.hpp)

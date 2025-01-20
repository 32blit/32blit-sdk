cmake_minimum_required(VERSION 3.12...3.31)
project (examples)
find_package (32BLIT CONFIG REQUIRED PATHS ..)

add_subdirectory(hardware-test)
add_subdirectory(picosystem-hardware-test)

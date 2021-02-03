cmake_minimum_required(VERSION 3.9)
project (another-world)
find_package (32BLIT CONFIG REQUIRED PATHS ../..)
blit_executable (another-world another-world.cpp virtual-machine.cpp resource.cpp)
blit_metadata (another-world metadata.yml)


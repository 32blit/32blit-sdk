#!/bin/bash
SDL2_BINPATH="/opt/sdl2-win64/bin"

# Awful helper to invoke cross compiling under WSL with specific SDL2 install path
# should probably be more flexible with sensible defaults and part of the Makefile

if [ ! -f `which x86_64-w64-mingw32-g++` ]; then
    echo "x86_64-w64-mingw32-g++ is required!"
    echo "1. Install: sudo apt install g++-mingw-w64-x86-64"
    exit 1
fi

if [ ! -d $SDL2_BINPATH ]; then
    echo "SDL2 missing from: \"$SDL2_BINPATH\""
    echo "1. Grab SDL source: hg clone https://hg.libsdl.org/SDL SDL"
    echo "2. Configure: ../configure --prefix=/opt/sdl2-win64 --host=x86_64-w64-mingw32"
    echo "3. Build: make"
    echo "4. Install: sudo make install"
    exit 1
fi

make OUTPUT_NAME=$1.exe PREFIX=x86_64-w64-mingw32- CXXFLAGS="-static-libstdc++ -static-libgcc -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic" SDL2_CONFIG=$SDL2_BINPATH/sdl2-config ../examples/$1
cp $SDL2_BINPATH/SDL2.dll build/$1.exe/
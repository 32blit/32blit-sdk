### Building & Running on Win32 (WSL + MinGW)

To build your project for Win32 you'll need `g++-mingw-w64` and `g++-mingw-w64`.

```
sudo apt-get install gcc-mingw-w64 g++-mingw-w64
```

You'll also need to cross-compile SDL2 and install it wherever you like to keep your cross-compile libraries.

```
wget https://www.libsdl.org/release/SDL2-2.0.10.zip
unzip SDL2-2.0.10.zip
cd SDL2-2.0.10
mkdir build.mingw
cd build.mingw
../configure --target=x86_64-w64-mingw32 --host=x86_64-w64-mingw32 --build=x86_64--linux --prefix=/usr/local/cross-tools/x86_64-w64-mingw32/
make
sudo make install
```

Finally, from the root directory of your project:

```
mkdir build.mingw
cd build.mingw
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../mingw.toolchain -DSDL2_DIR=/usr/local/cross-tools/x86_64-w64-mingw32/lib/cmake/SDL2
make
cp /usr/local/cross-tools/x86_64-w64-mingw32/bin/SDL2.dll .
```

# Building & Running on Win32 (WSL and MinGW)

These instructions cover setting up Windows Subsystem for Linux so that you can cross-compile Windows-compatible binaries with MinGW.

They assume a basic knowledge of the Linux command-line, installing tools and compiling code from source.

If you're more familiar with Visual Studio then you should [follow the instructions in Windows-VisualStudio.md](Windows-VisualStudio.md)

## Setting Up

### Windows Subsystem for Linux (WSL)

To enable Windows Subsystem for Linux, you must access the _Turn Windows features on or off_ dialog. Find the entry for _Windows Subsystem for Linux_ and make sure it is enabled.

After that, proceed to the Microsoft Store to download Ubuntu for WSL.

### Installing requirements inside WSL

The following requirements enable cross-compile to 32Blit via ARM GCC and to Windows via MinGW:

```shell
# Ubuntu 18.04's gcc-arm-none-eabi is too old, this repo backports the one from 19.10
sudo add-apt-repository ppa:daft-freak/arm-gcc
sudo apt update

sudo apt install gcc gcc-arm-none-eabi gcc-mingw-w64 g++-mingw-w64 unzip cmake make python3 python3-pip
pip3 install construct bitstring
```

## Building & Running on 32Blit

If you want to run code on 32Blit, you should now refer to [Building & Running On 32Blit](32blit.md).

## Building & Running Locally

You can use WSL on Windows to cross-compile your project (or any 32Blit example) into a Windows .exe for testing locally.

First you'll need to cross-compile SDL2 for MinGW and install it.

Grab the SDL2 source code and unzip it with the following commands:

```shell
wget https://www.libsdl.org/release/SDL2-2.0.10.zip
unzip SDL2-2.0.10.zip
cd SDL2-2.0.10
```

Then build and install it:

```shell
mkdir build.mingw
cd build.mingw
../configure --target=x86_64-w64-mingw32 --host=x86_64-w64-mingw32 --build=x86_64--linux --prefix=/usr/local/cross-tools/x86_64-w64-mingw32/
make
sudo make install
```

This will install the SDL2 development headers and libraries into `/usr/local/cross-tools/x86_64-w64-mingw32/` if you use a different directory then you will have to supply the SDL2 dir to the `cmake` command below using `-DSDL2_DIR=/usr/local/cross-tools/x86_64-w64-mingw32/lib/cmake/SDL2`

Finally, set up the 32Blit Makefile from the root of the repository with the following commands:

```shell
mkdir build.mingw
cd build.mingw
cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw.toolchain
```

Now to make any example, type:

```shell
make example-name
```

For example:

```shell
make raycaster
```

This will produce `examples/raycaster/raycaster.exe` which you should run with:

```shell
./examples/raycaster/raycaster.exe
```

WSL will launch the example in Windows, using the required `SDL2.dll` that will have been copied into the build root.

Don't forget to include `SDL2.dll` this if you want to redistribute a game/example.

### Build Everything

Alternatively you can build everything by just typing:

```shell
make
```

When the build completes you should be able to run any example.

### Troubleshooting

If you see `cannot create target because another target with the same name already exists` you've probably run `cmake ..` in the wrong directory (the project directory rather than the build directory), you should remove all but your project files and `cmake ..` again from the build directory.

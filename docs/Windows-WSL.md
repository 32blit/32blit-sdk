# Building & Running on Win32 (WSL or MinGW)

These instructions assume a basic knowledge of the Linux command-line, installing tools and compiling code from source.

If you're more familiar with Visual Studio then you should [follow the instructions in Windows-VisualStudio.md](Windows-VisualStudio.md)

## Setting Up

### Windows Subsystem for Linux (WSL)

To enable Windows Subsystem for Linux, you must access the _Turn Windows features on or off_ dialog. Find the entry for _Windows Subsystem for Linux_ and make sure it is enabled.

After that, proceed to the Microsoft Store to download Ubuntu for WSL.

### Installing requirements inside WSL

Install all the requirements using _apt_ and _pip3_ inside Ubuntu WSL:

```shell
sudo apt install gcc gcc-arm-none-eabi cmake make python3 python3-pip
pip3 install construct bitstring
```

If you want to deploy to the device, you will also need to download a DFU tool. On Windows it's easiest to just use "DfuSe Demonstration" (available from [st.com](https://www.st.com/en/development-tools/stsw-stm32080.html))

### XMing

To run the examples from WSL on Windows you will need to have XMing (or another XWindow Server) running on Windows. Click on the following link which will help you install and setup WSL and XMing together.

- [Information how to run XMing with WSL](https://virtualizationreview.com/articles/2017/02/08/graphical-programs-on-windows-subsystem-on-linux.aspx)
- [XMing homepage](http://www.straightrunning.com/XmingNotes/)
- Direct link to [download XMing setup](https://sourceforge.net/projects/xming/files/Xming/6.9.0.31/Xming-6-9-0-31-setup.exe/download)

You can then run code by either:
- prefixing the command with `DISPLAY=:0.0`, or 
- execute the command `export DISPLAY=:0.0` - which after you will not need to prefix the commands in the current session, just run them

## Build

The following sections contain information on how to build and run example projects.

### Building & Running on Linux or WSL + XMing

To build your project for testing, go into the relevant example directory. We'll use `palette-cycle` to demonstrate:

```shell
cd examples/palette-cycle
```

Prepare the Makefile with CMake:

```shell
mkdir build
cd build
cmake ..
```

And compile the example:

```shell
make
```

To run the application on your computer, use the following command (from within the same directory):

```shell
./palette-cycle
```

If you're using WSL and XMing you will need to add a `DISPLAY` variable like so:

```shell
DISPLAY=:0.0 ./palette-cycle
```

Note: Sound does not work with WSL/XMing (this is a WSL problem, not a 32Blit problem).

### Building & Running on Win32 (WSL + MinGW)

You can use WSL on Windows to cross-compile your project (or any 32Blit example) into a Windows .exe.

To build your project for Win32 you'll need `g++-mingw-w64`, `g++-mingw-w64` and `unzip` for extracting the SDL source code.

```shell
sudo apt-get install gcc-mingw-w64 g++-mingw-w64 unzip
```

You'll also need to cross-compile SDL2 and install it.

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

Finally, from the root directory of your project with the following commands:

```shell
mkdir build.mingw
cd build.mingw
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../mingw.toolchain
make
```

This will give you a `project-name.exe` in your build directory and will additionally copy the required `SDL2.dll`. Don't forget to include this if you want to share your game.

### Troubleshooting

If you see `cannot create target because another target with the same name already exists` you've probably run `cmake ..` in the wrong directory (the project directory rather than the build directory), you should remove all but your project files and `cmake ..` again from the build directory.

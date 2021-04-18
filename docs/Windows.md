# Building & Running on Windows <!-- omit in toc -->

These instructions cover building 32blit on Windows (without WSL).

- [Prerequisites](#prerequisites)
- [Using Visual Studio](#using-visual-studio)
- [Building & Running on 32Blit](#building--running-on-32blit)
- [Building & Running Locally](#building--running-locally)
  - [Build Everything](#build-everything)

## Prerequisites

You'll need to install:

 - [Git for Windows](https://git-scm.com/download/win)
 - [Python](https://www.python.org/downloads/)
 - At least the [Visual Studio build tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019) (For local builds, select "C++ build tools"). This includes a copy of CMake. A full Visual Studio install can also be used.
 - The [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) (For 32blit device builds, use the 9-2020-q2-update version). Make sure to select the "Add path to environment variable" at the end of the setup.

Now open "Developer Command Prompt for VS 2019" and install the 32blit tools:
```
py -m pip install 32blit
```
(Keep this open for building later)

This may result in a waning similar to:

```
WARNING: The script 32blit.exe is installed in 'C:\Users\[Name]\AppData\Local\Programs\Python\Python39\Scripts' which is not on PATH.
```

You will either need to add that to your PATH, or run the tools as `py -m ttblit ...` instead of `32blit ...`.

## Using Visual Studio
To build using Visual Studio [see here](Windows-VisualStudio.md). The rest of these instructions cover command-line builds.

## Building & Running on 32Blit

If you want to run code on 32Blit, you should now refer to [Building & Running On 32Blit](32blit.md). You will need to add `-G"NMake Makefiles"` to your cmake commands and use `nmake` instead of `make`. For example:
```
mkdir build.stm32
cd build.stm32
cmake -G"NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=../32blit.toolchain ..
nmake
```

## Building & Running Locally

Set up the 32Blit Makefile from the root of the repository with the following commands:

```shell
mkdir build
cd build
cmake -G"NMake Makefiles" ..
```

Now to make any example, type:

```
nmake example-name
```

For example:

```
nmake raycaster
```

This will produce `examples/raycaster/raycaster.exe` which you should run with:

```
.\examples\raycaster\raycaster
```

### Build Everything

Alternatively you can build everything by just typing:

```
nmake
```

When the build completes you should be able to run any example.

# Visual Studio 2019 

You can now also use Visual Studio 2019 to examine the samples (compile them and run them in an SDL window) or to build your own apps for 32blit. Remember to also test on device!

## Requirements

You will need Visual Studio 2019 (preferably version 16.4). 

Make sure you install C++ desktop development support.

The solutions and projects are made to use toolset version c142. 

## SDL libraries

You will also need to download SDL2 development libraries from the [SDL homepage](https://www.libsdl.org/download-2.0.php). Here find the latest version of the VC development libraries (at the time of this writing SDL2-devel-2.0.10-VC.zip).

Place these in the `vs\sdl\` folder.

## Visual Studio solution file

Now you can proceed to open `vs\32blit.sln`. It contains two static linked libraries, _32blit_ and _32blit-sdl_ and all the examples that will compile to .EXE. 

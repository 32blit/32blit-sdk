# SDL2 Binaries for Visual Studio Builds

Download the SDL2 development libraries from the [SDL homepage](https://www.libsdl.org/download-2.0.php). You should find the latest version of the VC development libraries (at the time of writing these are SDL2-devel-2.0.10-VC.zip).

Also, Download the SDL2_image development libraries from the [SDL_image homepage](https://www.libsdl.org/projects/SDL_image/). (At the time of writing the latest version is SDL2_image-devel-2.0.5-VC.zip).

Once extracted you should ensure the SDL "include" and "lib" directories are directly in `vs/sdl`. The tree should look like:

* vs/sdl
    * vs/sdl/include
    * vs/sdl/lib

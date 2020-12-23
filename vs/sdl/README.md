# SDL2 Binaries for Visual Studio Builds

You will need SDL2 development libraries and the SDL2_image library. 

* Download SDL2 development libraries from the [SDL homepage](https://www.libsdl.org/download-2.0.php). Here find the latest version of the VC development libraries (at the time of this writing *SDL2-devel-2.0.14-VC.zip*). 
* Download SDL2_image from [here](https://www.libsdl.org/projects/SDL_image/) (*SDL2_image-devel-2.0.5-VC.zip*).

Extract both into this folder. You will need to merge the include/lib directories.

Once extracted you should ensure the SDL "include" and "lib" directories are directly in `vs/sdl`. The tree should look like:

* vs/sdl
    * vs/sdl/include
    * vs/sdl/lib

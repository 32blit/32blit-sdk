if [ ! -f vs/sdl/include/SDL_image.h ]; then
    curl https://libsdl.org/release/SDL2-devel-2.0.14-VC.zip -o SDL2.zip
    curl https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.5-VC.zip -o SDL2_image.zip

    unzip SDL2.zip -d vs/sdl
    unzip SDL2_image.zip -d vs/sdl

    # move dirs up
    mv vs/sdl/SDL2-2.0.14/* vs/sdl
    cp -r vs/sdl/SDL2_image-2.0.5/* vs/sdl
fi
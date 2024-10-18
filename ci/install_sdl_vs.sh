if [ ! -f vs/sdl/include/SDL_net.h ]; then
    curl -L https://github.com/libsdl-org/SDL/releases/download/release-2.24.0/SDL2-devel-2.24.0-VC.zip -o SDL2.zip
    curl -L https://github.com/libsdl-org/SDL_net/releases/download/release-2.2.0/SDL2_net-devel-2.2.0-VC.zip -o SDL2_net.zip

    unzip SDL2.zip -d vs/sdl
    unzip -o SDL2_net.zip -d vs/sdl

    # move dirs up
    mv vs/sdl/SDL2-2.24.0/* vs/sdl
    cp -r vs/sdl/SDL2_net-2.2.0/* vs/sdl
fi

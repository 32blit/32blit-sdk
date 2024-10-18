if [ ! -f SDL2_net/README.txt ]; then
    curl -L https://github.com/libsdl-org/SDL/releases/download/release-2.24.0/SDL2-devel-2.24.0-mingw.tar.gz -o SDL2.tar.gz
    curl -L https://github.com/libsdl-org/SDL_net/releases/download/release-2.2.0/SDL2_net-devel-2.2.0-mingw.tar.gz -o SDL2_net.tar.gz

    tar -xf SDL2.tar.gz
    tar -xf SDL2_net.tar.gz

    # remove the version
    mv SDL2-2.24.0 SDL2
    mv SDL2_net-2.2.0 SDL2_net
fi

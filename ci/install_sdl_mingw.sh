if [ ! -f SDL2_net/README.txt ]; then
    curl -L https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-devel-2.28.5-mingw.tar.gz -o SDL2.tar.gz
    curl -L https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.12/SDL2_image-devel-2.8.12-mingw.tar.gz -o SDL2_image.tar.gz
    curl -L https://github.com/libsdl-org/SDL_net/releases/download/release-2.2.0/SDL2_net-devel-2.2.0-mingw.tar.gz -o SDL2_net.tar.gz

    tar -xf SDL2.tar.gz
    tar -xf SDL2_image.tar.gz
    tar -xf SDL2_net.tar.gz

    # remove the version
    mv SDL2-2.28.5 SDL2
    mv SDL2_image-2.8.12 SDL2_image
    mv SDL2_net-2.2.0 SDL2_net
fi
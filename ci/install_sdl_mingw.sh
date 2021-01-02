if [ ! -f SDL2_net-2.0.1/README.txt ]; then
    curl https://libsdl.org/release/SDL2-devel-2.0.10-mingw.tar.gz -o SDL2.tar.gz
    curl https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.5-mingw.tar.gz -o SDL2_image.tar.gz
    curl https://www.libsdl.org/projects/SDL_net/release/SDL2_net-devel-2.0.1-mingw.tar.gz -o SDL2_net.tar.gz

    tar -xf SDL2.tar.gz
    tar -xf SDL2_image.tar.gz
    tar -xf SDL2_net.tar.gz

    # fix the path in the cmake config
    sed -i "s|/opt/local|$PWD/SDL2-2.0.10|g" ./SDL2-2.0.10/x86_64-w64-mingw32/lib/cmake/SDL2/sdl2-config.cmake

    # copy SDL2_image/net into SDL2
    cp -r ./SDL2_image-2.0.5/x86_64-w64-mingw32 ./SDL2-2.0.10/
    cp -r ./SDL2_net-2.0.1/x86_64-w64-mingw32 ./SDL2-2.0.10/
fi
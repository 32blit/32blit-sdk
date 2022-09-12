if [ ! -f ~/Library/Frameworks/SDL2_net.framework/SDL2_net ]; then
    curl -L https://github.com/libsdl-org/SDL/releases/download/release-2.24.0/SDL2-2.24.0.dmg -o SDL2.dmg
    curl -L https://github.com/libsdl-org/SDL_image/releases/download/release-2.6.2/SDL2_image-2.6.2.dmg -o SDL2_image.dmg
    curl -L https://github.com/libsdl-org/SDL_net/releases/download/release-2.2.0/SDL2_net-2.2.0.dmg -o SDL2_net.dmg

    hdiutil mount SDL2.dmg
    hdiutil mount SDL2_image.dmg
    hdiutil mount SDL2_net.dmg

    mkdir -p ~/Library/Frameworks
    cp -r /Volumes/SDL2/SDL2.framework ~/Library/Frameworks/
    cp -r /Volumes/SDL2_image/SDL2_image.framework ~/Library/Frameworks/
    cp -r /Volumes/SDL2_net/SDL2_net.framework ~/Library/Frameworks/
fi
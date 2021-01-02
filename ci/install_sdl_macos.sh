if [ ! -f ~/Library/Frameworks/SDL2_net.framework/SDL2_net ]; then
    curl https://libsdl.org/release/SDL2-2.0.10.dmg -o SDL2.dmg
    curl http://libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.dmg -o SDL2_image.dmg
    curl http://libsdl.org/projects/SDL_net/release/SDL2_net-2.0.1.dmg -o SDL2_net.dmg

    hdiutil mount SDL2.dmg
    hdiutil mount SDL2_image.dmg
    hdiutil mount SDL2_net.dmg

    mkdir -p ~/Library/Frameworks
    cp -r /Volumes/SDL2/SDL2.framework ~/Library/Frameworks/
    cp -r /Volumes/SDL2_image/SDL2_image.framework ~/Library/Frameworks/
    cp -r /Volumes/SDL2_net/SDL2_net.framework ~/Library/Frameworks/
fi
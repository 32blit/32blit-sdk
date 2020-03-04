#include "palette-cycle.hpp"

using namespace blit;

struct cycleg {
    uint8_t offset;
    float speed;
    bool reverse;
    Pen colours[12];
};

cycleg cycle_gradients[3] = {
    {16, 100, true, {}},
    {32, 200, true, {}},
    {48, 300, false, {}},
};

Pen fadeColour(Pen a, Pen b, float f) {
    Pen t(0, 0, 0, 255);
    t.r = std::min(255, std::max(0, int(a.r + (float(b.r - a.r) * f))));
    t.g = std::min(255, std::max(0, int(a.g + (float(b.g - a.g) * f))));
    t.b = std::min(255, std::max(0, int(a.b + (float(b.b - a.b) * f))));
    return t;
}

void init() {
    set_screen_mode(ScreenMode::hires);

    screen.sprites = SpriteSheet::load(fire_data);

    for(int g = 0; g < 3; g++){
        for(int i = 0; i < 12; i++){
            Pen c = screen.sprites->palette[cycle_gradients[g].offset + i];
            cycle_gradients[g].colours[i] = c;
        }
    }
}

void cycle(uint32_t time) {
    for(int g = 0; g < 3; g++){
        int offset = cycle_gradients[g].offset;
        float speed = cycle_gradients[g].speed;
        float t = time / speed;
        float phase = (t - floorf(t));
        for(int i = 0; i < 12; i++){
            int index = i + t;
            index %= 12;
            int blend_to = index + 1;
            if (blend_to > 11) {blend_to = 0;}
            if (blend_to < 0) {blend_to = 11;}

            int target = i;
            if(cycle_gradients[g].reverse){
                target = 11 - target;
            }

            screen.sprites->palette[offset + target] = fadeColour(
                cycle_gradients[g].colours[index],
                cycle_gradients[g].colours[blend_to],
                std::max(0.0f, std::min(1.0f, phase)));
        }
    }
}

void render(uint32_t time) {
    cycle(time);

    screen.pen = screen.sprites->palette[0x59];
    screen.clear();

    screen.alpha = 255;
    screen.mask = nullptr;
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(0, 0, 320, 14));
    screen.pen = Pen(0, 0, 0);
    screen.text("Palette cycle demo", minimal_font, Point(5, 4));

    int upscaled_size = 64 * 2;
    screen.stretch_blit(screen.sprites, Rect(0, 0, 64, 64), Rect(4 + screen.bounds.w / 2, (screen.bounds.h - upscaled_size) / 2, upscaled_size, upscaled_size));

    int swatch_size = 8;
    for (int y = 0; y < 6; y++){
        for (int x = 0; x < 16; x++) {
            int ox = 28 + (x * swatch_size);
            int oy = (screen.bounds.h - upscaled_size) / 2 + (y * swatch_size);
            int i = x + y * 16;
            Pen col = screen.sprites->palette[i];
            screen.pen = col;
            screen.rectangle(Rect(
                ox, oy, swatch_size, swatch_size
            ));
        }
    }

    screen.watermark();
}

void update(uint32_t time) {
}
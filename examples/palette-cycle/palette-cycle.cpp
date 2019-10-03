#include "palette-cycle.hpp"

using namespace blit;

struct cycleg {
    uint8_t offset;
    float speed;
    bool reverse;
    rgba colours[12];
};

cycleg cycle_gradients[3] = {
    {16, 100, true, {}},
    {32, 200, true, {}},
    {48, 300, false, {}},
};

rgba fadeColour(rgba a, rgba b, float f) {
    rgba t(0, 0, 0, 255);
    t.r = std::min(255, std::max(0, int(a.r + (float(b.r - a.r) * f))));
    t.g = std::min(255, std::max(0, int(a.g + (float(b.g - a.g) * f))));
    t.b = std::min(255, std::max(0, int(a.b + (float(b.b - a.b) * f))));
    return t;
}

void init() {
    set_screen_mode(screen_mode::hires);

    fb.sprites = spritesheet::load(fire_data);

    for(int g = 0; g < 3; g++){
        for(int i = 0; i < 12; i++){
            rgba c = fb.sprites->palette[cycle_gradients[g].offset + i];
            cycle_gradients[g].colours[i] = c;
        }
    }
}

void cycle(uint32_t time) {
    for(int g = 0; g < 3; g++){
        int offset = cycle_gradients[g].offset;
        float speed = cycle_gradients[g].speed;
        float t = time / speed;
        float phase = (t - floor(t));
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

            fb.sprites->palette[offset + target] = fadeColour(
                cycle_gradients[g].colours[index],
                cycle_gradients[g].colours[blend_to],
                std::max(0.0f, std::min(1.0f, phase)));
        }
    }
}

void render(uint32_t time) {
    cycle(time);

    fb.pen(fb.sprites->palette[0x59]);
    fb.clear();

    fb.alpha = 255;
    fb.mask = nullptr;
    fb.pen(rgba(255, 255, 255));
    fb.rectangle(rect(0, 0, 320, 14));
    fb.pen(rgba(0, 0, 0));
    fb.text("Palette cycle demo", &minimal_font[0][0], point(5, 4));

    int upscaled_size = 64 * 2;
    fb.stretch_blit(fb.sprites, rect(0, 0, 64, 64), rect(4 + fb.bounds.w / 2, (fb.bounds.h - upscaled_size) / 2, upscaled_size, upscaled_size));

    int swatch_size = 8;
    for (int y = 0; y < 6; y++){
        for (int x = 0; x < 16; x++) {
            int ox = 28 + (x * swatch_size);
            int oy = (fb.bounds.h - upscaled_size) / 2 + (y * swatch_size);
            int i = x + y * 16;
            rgba col = fb.sprites->palette[i];
            fb.pen(col);
            fb.rectangle(rect(
                ox, oy, swatch_size, swatch_size
            ));
        }
    }

    fb.watermark();
}

void update(uint32_t time) {
}
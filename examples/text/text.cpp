#include "text.hpp"

bool variable_width = true;
uint32_t prev_buttons = blit::buttons;
blit::text_align alignment = blit::text_align::top_left;

void init() {
    blit::set_screen_mode(blit::screen_mode::lores);
}

void render(uint32_t time) {
    blit::fb.pen(blit::rgba(0x80, 0x80, 0x80));
    blit::fb.clear();

    blit::rect text_rect(20, 20, 120, 80);

    blit::fb.pen(blit::rgba(0xFF));
    blit::fb.rectangle(text_rect);

    std::string text = "This is some aligned text!\nUse the dpad to change the alignment\nand A to toggle variable-width.";
    text = blit::fb.wrap_text(text, text_rect.w, &minimal_font[0][0], variable_width);

    blit::fb.pen(blit::rgba(0xFF, 0xFF, 0xFF));
    blit::fb.text(text, &minimal_font[0][0], text_rect, variable_width, alignment);

    auto size = blit::fb.measure_text(text, &minimal_font[0][0], variable_width);
    blit::fb.text("bounds: " + std::to_string(size.w) + "x" + std::to_string(size.h), &minimal_font[0][0], blit::point(0, 0));
}

void update(uint32_t time) {
    if ((prev_buttons & blit::button::A) && !(blit::buttons & blit::button::A))
        variable_width = !variable_width;

    alignment = blit::text_align::top_left;

    if (blit::buttons & blit::button::DPAD_DOWN)
        alignment = (blit::text_align)(alignment | blit::text_align::bottom);
    else if (!(blit::buttons & blit::button::DPAD_UP))
        alignment = (blit::text_align)(alignment | blit::text_align::center_v);

    if (blit::buttons & blit::button::DPAD_RIGHT)
        alignment = (blit::text_align)(alignment | blit::text_align::right);
    else if (!(blit::buttons & blit::button::DPAD_LEFT))
        alignment = (blit::text_align)(alignment | blit::text_align::center_h);

    prev_buttons = blit::buttons;
}
#include "text.hpp"

using namespace blit;

bool variable_width = true;
uint32_t prev_buttons = buttons;
TextAlign alignment = TextAlign::top_left;

void init() {
    set_screen_mode(ScreenMode::lores);
}

void render(uint32_t time) {
    screen.pen(RGBA(0x80, 0x80, 0x80));
    screen.clear();

    Rect text_rect(20, 20, 120, 80);

    screen.pen(RGBA(0xFF));
    screen.rectangle(text_rect);

    std::string text = "This is some aligned text!\nUse the dpad to change the alignment\nand A to toggle variable-width.";
    text = screen.wrap_text(text, text_rect.w, &minimal_font[0][0], variable_width);

    screen.pen(RGBA(0xFF, 0xFF, 0xFF));
    screen.text(text, &minimal_font[0][0], text_rect, variable_width, alignment);

    auto size = screen.measure_text(text, &minimal_font[0][0], variable_width);
    screen.text("bounds: " + std::to_string(size.w) + "x" + std::to_string(size.h), &minimal_font[0][0], Point(80, 0), true, TextAlign::center_h);
}

void update(uint32_t time) {
    if ((prev_buttons & Button::A) && !(buttons & Button::A))
        variable_width = !variable_width;

    alignment = TextAlign::top_left;

    if (buttons & Button::DPAD_DOWN)
        alignment = (TextAlign)(alignment | TextAlign::bottom);
    else if (!(buttons & Button::DPAD_UP))
        alignment = (TextAlign)(alignment | TextAlign::center_v);

    if (buttons & Button::DPAD_RIGHT)
        alignment = (TextAlign)(alignment | TextAlign::right);
    else if (!(buttons & Button::DPAD_LEFT))
        alignment = (TextAlign)(alignment | TextAlign::center_h);

    prev_buttons = buttons;
}

#include "text.hpp"

using namespace blit;

bool variable_width = true;
uint32_t prev_buttons = buttons;
TextAlign alignment = TextAlign::top_left;

std::string alignment_to_string(TextAlign alignment) {
	switch (alignment) {
		case TextAlign::bottom_left:
			return "align: bottom_left";
		case TextAlign::bottom_right:
			return "align: bottom_right";
		case TextAlign::top_left:
			return "align: top_left";
		case TextAlign::top_right:
			return "align: top_right";
		case TextAlign::center_center:
			return "align: center_center";
		case TextAlign::center_left:
			return "align: center_left";
		case TextAlign::center_right:
			return "align: center_right";
		case TextAlign::top_center:
			return "align: top_center";
		case TextAlign::bottom_center:
			return "align: bottom_center";
	}
	return "";
}

void init() {
    set_screen_mode(ScreenMode::hires);
}

void render(uint32_t time) {
	screen.pen(RGBA(0, 0, 0));
	screen.clear();

	screen.alpha = 255;
	screen.pen(RGBA(255, 255, 255));
	screen.rectangle(Rect(0, 0, 320, 14));
	screen.pen(RGBA(0, 0, 0));
	screen.text("Text Rendering", &minimal_font[0][0], Point(5, 4));

    // alignment
    Rect text_rect(20, 20, 120, 80);

    screen.pen(RGBA(64, 64, 64));
    screen.rectangle(text_rect);

	screen.pen(RGBA(255, 255, 255));
    std::string text = "This is some aligned text!\nUse the dpad to change the alignment\nand A to toggle variable-width.";
    text = screen.wrap_text(text, text_rect.w, &minimal_font[0][0], variable_width);

    screen.pen(RGBA(0xFF, 0xFF, 0xFF));
    screen.text(text, &minimal_font[0][0], text_rect, variable_width, alignment);

	screen.text(alignment_to_string(alignment), &minimal_font[0][0], Point(80, 102), true, TextAlign::center_h);

    auto size = screen.measure_text(text, &minimal_font[0][0], variable_width);
    screen.text("bounds: " + std::to_string(size.w) + "x" + std::to_string(size.h), &minimal_font[0][0], Point(80, 110), true, TextAlign::center_h);

    text_rect.x += 160;

    // clipping
    Rect clip(text_rect.x + 30 + 30 * cos(time / 1000.0f), text_rect.y, 60, 80);
    screen.pen(RGBA(64, 64, 64));
    screen.rectangle(text_rect);

    text = "This text is clipped!\nIt's slightly hard to read since half of it is missing.";
    text = screen.wrap_text(text, text_rect.w, &minimal_font[0][0], variable_width);

    screen.pen(RGBA(0xFF, 0xFF, 0xFF));
    screen.text(text, &minimal_font[0][0], text_rect, variable_width, TextAlign::center_center, clip);
}

void update(uint32_t time) {
    if ((prev_buttons & Button::A) && !(buttons & Button::A))
        variable_width = !variable_width;

    alignment = TextAlign::top_left;

	if (buttons & Button::DPAD_DOWN) {
		alignment = (TextAlign)(alignment | TextAlign::bottom);
	}
	else if (!(buttons & Button::DPAD_UP)) {
		alignment = (TextAlign)(alignment | TextAlign::center_v);
	}

	if (buttons & Button::DPAD_RIGHT) {
		alignment = (TextAlign)(alignment | TextAlign::right);
	}
	else if (!(buttons & Button::DPAD_LEFT)) {
		alignment = (TextAlign)(alignment | TextAlign::center_h);
	}

    prev_buttons = buttons;
}

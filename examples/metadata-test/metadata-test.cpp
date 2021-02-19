#include "metadata-test.hpp"

using namespace blit;

GameMetadata metadata;
std::string description;

void init() {
  set_screen_mode(hires);
  metadata = get_metadata();
  description = screen.wrap_text(metadata.description, 220, minimal_font);
}

void update(uint32_t time) {

}

void render(uint32_t time) {
  screen.pen = Pen(20, 30, 40);
  screen.clear();

  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));
  screen.pen = Pen(0, 0, 0);
  screen.text("Metadata Test", minimal_font, Point(5, 4));

  screen.pen = Pen(80, 120, 160);

  screen.text("Title:", minimal_font, Point(5, 24));
  screen.text("Desc:", minimal_font, Point(5, 44));
  screen.text("Author:", minimal_font, Point(5, 104));
  screen.text("Version:", minimal_font, Point(5, 124));
  screen.text("URL:", minimal_font, Point(5, 144));
  screen.text("Category:", minimal_font, Point(5, 164));

  screen.pen = Pen(255, 255, 255);

  screen.text(metadata.title, minimal_font, Point(80, 24));
  screen.text(description, minimal_font, Rect(80, 44, 220, 40));
  screen.text(metadata.author, minimal_font, Point(80, 104));
  screen.text(metadata.version, minimal_font, Point(80, 124));
  screen.text(metadata.url, minimal_font, Point(80, 144));
  screen.text(metadata.category, minimal_font, Point(80, 164));

  screen.watermark();
}
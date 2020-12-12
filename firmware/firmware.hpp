#include "32blit.h"
#include "32blit.hpp"
#include "CDCCommandHandler.h"
#include "fatfs.h"
#include "persistence.h"

#define BUFFER_SIZE (256)
#define MAX_FILENAME 256+1
#define MAX_FILELEN 16+1
#define PAGE_SIZE 256

using namespace blit;

// progress bar
struct {
  std::string message;
  int32_t value = 0;
  int32_t total = 0;

  void show(std::string message, uint32_t total) {
    this->message = message;
    this->total = total;
    this->value = 0;
    render_yield();
  }

  void hide() {
    this->message = "";
    render_yield();
  }

  void update(uint32_t value) {
    this->value = value;
    render_yield();
  }

  void draw() {
    if(!this->message.empty()) {
      screen.pen = Pen(0, 0, 0, 150);
      screen.rectangle(Rect(0, 215, 320, 25));
      screen.pen = Pen(255, 255, 255);
      screen.text(this->message, minimal_font, Point(5, 220));
      uint32_t progress_width = ((this->value * 310) / this->total);      
      screen.rectangle(Rect(5, 230, progress_width, 5));
    }
  }
} progress;

struct {
  std::string title, message;

  using AnswerFunc = void(*)(bool);
  AnswerFunc on_answer;

  void show(std::string title, std::string message, AnswerFunc on_answer) {
    this->title = title;
    this->message = message;
    this->on_answer = on_answer;
  }

  bool update() {
    if(title.empty())
      return false;

    if(buttons.released & Button::A) {
      on_answer(true);
      title = message = "";
    } else if(buttons.released & Button::B) {
      on_answer(false);
      title = message = "";
    }

    return true;
  }

  void draw() {
    if(title.empty())
      return;

    const Rect dialog_rect(45, 77, 230, 85);
    const int header_height = 16;

    screen.pen = Pen(235, 245, 255);
    screen.rectangle(dialog_rect);

    screen.pen = Pen(3, 5, 7);
    screen.rectangle(Rect(dialog_rect.x + 1, dialog_rect.y + header_height, dialog_rect.w - 2, dialog_rect.h - header_height - 1));

    screen.text(title, minimal_font, Rect(dialog_rect.x + 3, dialog_rect.y, dialog_rect.w - 6, header_height + minimal_font.spacing_y), true, TextAlign::center_left);

    screen.pen = Pen(255, 255, 255);
    screen.text(message, minimal_font, Rect(dialog_rect.x + 6, dialog_rect.y + header_height + 5, dialog_rect.w - 12, 45));

    screen.text("No      Yes    ", minimal_font, Rect(dialog_rect.x + 1, dialog_rect.y + dialog_rect.h - 17, dialog_rect.w - 2, 16 + minimal_font.spacing_y), true, TextAlign::center_right);
  
    screen.sprite(0, Point(dialog_rect.x + 185, dialog_rect.y + 71), SpriteTransform::R180);
    screen.sprite(0, Point(dialog_rect.x + 218, dialog_rect.y + 71), SpriteTransform::R90);
  }

} dialog;

class CDCEraseHandler final : public CDCCommandHandler {
public:
  StreamResult StreamData(CDCDataStream &dataStream) override;

  bool StreamInit(CDCFourCC uCommand) override {
    return true;
  }
};

class FlashLoader : public CDCCommandHandler
{
public:
  virtual StreamResult StreamData(CDCDataStream &dataStream);
  virtual bool StreamInit(CDCFourCC uCommand);

private:
  enum ParseState {stFilename, stLength, stRelocs, stData};

  ParseState m_parseState = stFilename;

  char m_sFilename[MAX_FILENAME];
  char m_sFilelen[MAX_FILELEN];

  uint32_t m_uParseIndex = 0;
  uint32_t m_uFilelen = 0;
  uint32_t flash_start_offset = 0;

  uint32_t num_relocs = 0, cur_reloc = 0;
  std::vector<uint32_t> relocation_offsets;
};

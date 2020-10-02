#include "32blit.h"
#include "32blit.hpp"
#include "CDCCommandHandler.h"
#include "fatfs.h"
#include "persistence.h"

#define BUFFER_SIZE (256)
#define ROW_HEIGHT 10
#define ROW(x) Point(0,x * ROW_HEIGHT)
#define MAX_FILENAME 256+1
#define MAX_FILELEN 16+1
#define PAGE_SIZE 256

using namespace blit;

enum class SortBy {
  name,
  size
};

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

class FlashLoader : public CDCCommandHandler
{
public:
  virtual StreamResult StreamData(CDCDataStream &dataStream);
  virtual bool StreamInit(CDCFourCC uCommand);

private:
  enum ParseState {stFilename, stLength, stData};

  ParseState m_parseState = stFilename;

  char m_sFilename[MAX_FILENAME];
  char m_sFilelen[MAX_FILELEN];

  uint32_t m_uParseIndex = 0;
  uint32_t m_uFilelen = 0;
  uint32_t flash_start_offset = 0;
};

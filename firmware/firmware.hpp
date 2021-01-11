#include "32blit.h"
#include "32blit.hpp"
#include "CDCCommandHandler.h"
#include "fatfs.h"
#include "persistence.h"

#define BUFFER_SIZE (256)
#define MAX_FILENAME 256+1
#define MAX_FILELEN 16+1
#define PAGE_SIZE 256
#define SD_BUFFER_SIZE 4096 // buffer size for flashing from SD

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

  enum State {stFlashFile, stSaveFile, stFlashCDC, stMassStorage};

  State state = stFlashFile;

  FIL file;
  uint8_t buffer[PAGE_SIZE];

  ParseState m_parseState = stFilename;

  char m_sFilename[MAX_FILENAME];
  char m_sFilelen[MAX_FILELEN];

  uint32_t m_uParseIndex = 0;
  uint32_t m_uFilelen = 0;
  uint32_t flash_start_offset = 0;

  uint32_t num_relocs = 0, cur_reloc = 0;
  std::vector<uint32_t> relocation_offsets;
  bool flash_mapped = false;
};

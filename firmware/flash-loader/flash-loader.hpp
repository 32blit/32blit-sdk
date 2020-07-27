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


  void Init(void);
  void Render(uint32_t time);
  void Update(uint32_t time);

private:
  enum State {stFlashFile, stSaveFile, stFlashCDC, stLS, stSwitch, stMassStorage};
  enum ParseState {stFilename, stLength, stData};

  bool Flash(const char *pszFilename);
  void FSInit(void);

  void RenderFlashFile(uint32_t time);
  void RenderMassStorage(uint32_t time);

  bool FlashData(uint32_t uOffset, uint8_t *pBuffer, uint32_t uLen);
  bool SaveData(uint8_t *pBuffer, uint32_t uLen);

  std::vector<blit::FileInfo> m_filemeta;
  int32_t m_max_width_size = 0;

  uint8_t m_buffer[PAGE_SIZE];
  uint8_t m_verifyBuffer[PAGE_SIZE];

  unsigned m_uCurrentFile = 0;
  bool		m_bFsInit = false;
  State		m_state = stFlashFile;

  ParseState m_parseState = stFilename;

  char m_sFilename[MAX_FILENAME];
  char m_sFilelen[MAX_FILELEN];

  uint32_t m_uParseIndex = 0;
  uint32_t m_uFilelen = 0;

  FIL m_file;

  float m_fPercent;

};

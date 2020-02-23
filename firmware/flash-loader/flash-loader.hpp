#include <string>
#include <vector>

#include "32blit.h"
#include "32blit.hpp"

#include "fatfs.h"
#include "persistence.h"

using namespace blit;

extern std::vector<FileInfo> files;

bool flash_from_sd_to_qspi_flash(const std::string &filename);

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
    if(this->message != "") {
      screen.pen = Pen(0, 0, 0, 150);
		  screen.rectangle(Rect(0, 215, 320, 25));
      screen.pen = Pen(255, 255, 255);
      screen.text(this->message, minimal_font, Point(5, 220));
      uint32_t progress_width = ((this->value * 310) / this->total);      
      screen.rectangle(Rect(5, 230, progress_width, 5));
    }
  }
} progress;

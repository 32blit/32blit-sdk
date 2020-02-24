/*
  32blit firmware. shows main menu and handles file transfers.

  this work is based on heavily on the original concept implemented
  by andrewcapon. any genius may safely be attributed to him while 
  any faults can be attributed to lowfatcode.

  author: lowfatcode (standing on the shoulders of andrewcapon)
  created: 23rd feb 2020
*/


#include "flash-loader.hpp"
#include "graphics/color.hpp"
#include <cmath>
#include "quadspi.h"
#include "usbd_def.h"
#include "usb-serial.hpp"
#include "core-debug.hpp"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

using namespace blit;
using namespace usb_serial;
 
constexpr uint32_t qspi_flash_sector_size = 64 * 1024;

std::vector<FileInfo> files;
Vec2 file_list_scroll_offset(20.0f, 0.0f);

inline bool ends_with(std::string const &value, std::string const &ending) {
  if(ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void load_file_list() {
  files.clear();
  for(auto file : list_files("")) {
    if(ends_with(file.name, ".bin")) {
      files.push_back(file);
    }
  }
  sort(files.begin(), files.end()); 
}

void select_file(const char *filename) {
  for(size_t i = 0; i < files.size(); i++) {
    if(files[i].name == filename) {
      persist.selected_menu_item = i;
    }
  }
}

Vec2 list_offset(5.0f, 0.0f);

CommandState usb_cdc_save_to_sd_card(CommandState state, char *data, uint32_t length);

CommandState usb_cdc_reset(CommandState state, char *data, uint32_t length) {
  NVIC_SystemReset();
  return CommandState::END;
}

CommandState usb_cdc_application_location(CommandState state, char *data, uint32_t length) {
  while(USBD_BUSY == transmit("32BL_EXT")) {};
	HAL_Delay(250);
	return CommandState::END;
}


void init()
{
  set_screen_mode(ScreenMode::hires);
  load_file_list();

  register_command_handler("RESET", usb_cdc_reset);
  register_command_handler("INFO", usb_cdc_application_location);
  register_command_handler("SAVE", usb_cdc_save_to_sd_card);
}

void background(uint32_t time_ms) {  
  constexpr uint8_t blob_count = 50;
  static Vec3 blobs[blob_count];

  static bool first = true;
/*
  for(uint16_t y = 0; y < 420; y++) {
    for(uint16_t x = 0; x < 320; x++) {
      screen.pen = Pen(x >> 1, y, 255 - (x >> 1));
      screen.pixel(Point(x,y));
    }
  }
//  constexpr float pi = 3.1415927;

  if(first) {
    for(auto &blob : blobs) {
      blob.x = (random() % 320);
      blob.y = (random() % 240);
      blob.z = (random() % 20) + 20;
    }
    first = false;
  }

  for(uint8_t i = 0; i < blob_count; i++) {
    float step = (time_ms + i * 200) / 1000.0f;
    Vec3 &blob = blobs[i];
    screen.pen = Pen(0, 0, 0, 20);
    int x = blob.x + sin(step) * i * 2.0f;
    int y = blob.y + cos(step) * i * 2.0f;
    int r = blob.z;// * (sin(step) + cos(step) + 1.0f);
    screen.circle(Point(x, y), r);
  }
  */
}

void render(uint32_t time_ms)
{
  screen.pen = Pen(5, 8, 12);
  screen.clear();
  
  background(time_ms);

  screen.pen = Pen(0, 0, 0, 100);
  screen.rectangle(Rect(10, 0, 100, 240));

  for(uint32_t i = 0; i < files.size(); i++) {
    FileInfo *file = &files[i];

    screen.pen = Pen(80, 100, 120);
    if(i == persist.selected_menu_item) {
      screen.pen = Pen(235, 245, 255);
    }
    std::string display_name = file->name.substr(0, file->name.size() - 4);
    screen.text(display_name, minimal_font, Point(file_list_scroll_offset.x, 115 + i * 10 - file_list_scroll_offset.y));
  }

  screen.watermark();
  
  progress.draw();
  error.draw();
}


int32_t modulo(int32_t x, int32_t n) {
  return (x % n + n) % n;
}

void update(uint32_t time)
{
  static uint32_t last_buttons;
  static uint32_t up_repeat = 0;
  static uint32_t down_repeat = 0;

  bool up_pressed = (buttons & DPAD_UP) && !(last_buttons & DPAD_UP);
  bool down_pressed = (buttons & DPAD_DOWN) && !(last_buttons & DPAD_DOWN);

  up_repeat = (buttons & DPAD_UP) ? up_repeat + 1 : 0;
  down_repeat = (buttons & DPAD_DOWN) ? down_repeat + 1 : 0;

  uint8_t repeat_count = 25;
  if(up_repeat > repeat_count) {
    up_pressed = true;
    up_repeat = 0;
  }

  if(down_repeat > repeat_count) {
    down_pressed = true;
    down_repeat = 0;
  }

  bool a_pressed = (buttons & A) && !(last_buttons & A);

  // handle up/down clicks to select files in the list
  if(up_pressed)    { persist.selected_menu_item--; }
  if(down_pressed)  { persist.selected_menu_item++; }
  int32_t file_count = files.size();
  persist.selected_menu_item = modulo(persist.selected_menu_item, file_count);

  // scroll list towards selected item  
  file_list_scroll_offset.y += ((persist.selected_menu_item * 10) - file_list_scroll_offset.y) / 5.0f;

  // select current item in list to launch
  if(a_pressed) {
    flash_from_sd_to_qspi_flash(files[persist.selected_menu_item].name);
    blit_switch_execution();
  }

  last_buttons = buttons;
}




bool flash_from_sd_to_qspi_flash(const std::string &filename)
{  
  static uint8_t file_buffer[4 * 1024];

	FIL file;
	if(f_open(&file, filename.c_str(), FA_READ) != FR_OK) {
		return false;
	}

  UINT bytes_total = f_size(&file);  

  // erase the sectors needed to write the image  
  uint32_t sector_count = (bytes_total / qspi_flash_sector_size) + 1;

  progress.show("Erasing flash sectors...", sector_count);

  for(uint32_t sector = 0; sector < sector_count; sector++) {
    qspi_sector_erase(sector * qspi_flash_sector_size);

    progress.update(sector);    
  }

  // read the image from as card and write it to the qspi flash  
  progress.show("Copying from SD card to flash...", bytes_total);

  UINT bytes_flashed  = 0;
  
  while(bytes_flashed < bytes_total) {
    UINT bytes_read = 0;
    
    // read up to 4KB from the file on sd card
    if(f_read(&file, (void *)file_buffer, sizeof(file_buffer), &bytes_read) != FR_OK) {
      return false;
    }

    // write the read data to the external qspi flash
    if(qspi_write_buffer(bytes_flashed, file_buffer, bytes_read) != QSPI_OK) {
      return false;
    }

    bytes_flashed += bytes_read;

    // TODO: is it worth reading the data back and performing a verify here? not sure...

    progress.update(bytes_flashed);
  }

  f_close(&file);

  progress.hide();

	return true;
}


// when a command is issued (e.g. "PROG" or "SAVE") then this function is called
// whenever new data is received to allow the firmware to process it.
CommandState usb_cdc_save_to_sd_card(CommandState state, char *data, uint32_t length) {
  static char   filename[64] = {'\0'};
  static FIL    file;
  
  if(state == CommandState::TIMEOUT) {    
    // if the stream times out then clean up ready for another go
    std::string message = std::string("Writing file '") + filename + std::string("' to SD card timed out.");
    error.show(message);        
    progress.hide();
    f_close(&file);
    memset(filename, 0, sizeof(filename));    
    return CommandState::ERROR;
  }

  if(state == CommandState::STREAM) {    
    static UINT bytes_total = 0;
    static UINT bytes_processed = 0;

    if(strlen(filename) == 0) {
      // extract filename
      strcpy(filename, data);
      length -= strlen(data) + 1;
      data += strlen(data) + 1;
      
      // extract length in bytes
      bytes_total = atol(data);
      length -= strlen(data) + 1;
      data += strlen(data) + 1;

      // reset processed bytes counter
      bytes_processed = 0;

      // open the file for writing
      if(f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {       
        std::string message = std::string("Couldn't open file '") + filename + std::string("' for writing on SD card.");
        error.show(message);        
        f_close(&file);     
        return CommandState::ERROR;
      }
      
      // setup progress bar
      progress.show("Copying to SD card...", bytes_total);
    }

    if(length > 0) {  
      UINT bytes_written;
      
      if(f_write(&file, data, length, &bytes_written) != FR_OK) {
        std::string message = std::string("Couldn't write to file '") + filename + std::string("' on SD card.");
        error.show(message);        
        f_close(&file);
        return CommandState::ERROR;        
      }
      
      bytes_processed += bytes_written;

      // completed the write? close the file
      if(bytes_processed == bytes_total) {
        progress.hide();
        load_file_list();
        select_file(filename);
        f_close(&file);
        memset(filename, 0, sizeof(filename));
        return CommandState::END;
      }else{
        progress.update(bytes_processed);        
      }
    }

    return CommandState::STREAM;
  }  

  return CommandState::ERROR;
}

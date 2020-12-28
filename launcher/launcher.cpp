#include <cmath>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <list>

#include "launcher.hpp"
#include "assets.hpp"

#include "engine/api_private.hpp"
#include "graphics/color.hpp"

#include "executable.hpp"
#include "metadata.hpp"
#include "dialog.hpp"

Dialog dialog;

struct Persist {
  unsigned int selected_menu_item = 0;
};
Persist persist;

using namespace blit;

constexpr uint32_t qspi_flash_sector_size = 64 * 1024;

bool sd_detected = true;
Vec2 file_list_scroll_offset(20.0f, 0.0f);
float directory_list_scroll_offset = 0.0f;

struct GameInfo {
  std::string title;
  uint32_t size, checksum = 0;

  std::string filename; // if on SD
};

struct DirectoryInfo {
  std::string name;
  int x, w;
};

std::vector<GameInfo> game_list;
std::list<DirectoryInfo> directory_list;
std::list<DirectoryInfo>::iterator current_directory;

SortBy file_sort = SortBy::name;

BlitGameMetadata selected_game_metadata;

SpriteSheet *spritesheet;

int calc_num_blocks(uint32_t size) {
  return (size - 1) / qspi_flash_sector_size + 1;
}

// insertion sort
template <class Iterator, class Compare>
void insertion_sort(Iterator first, Iterator last, Compare comp) {
  if(last - first < 2)
    return;

  for(auto it = first + 1; it != last; ++it) {
    auto temp = it;

    while(temp != first && comp(*temp, *(temp - 1))) {
      std::swap(*temp, *(temp - 1));
      --temp;
    }
  }
}

void sort_file_list() {
    using Iterator = std::vector<GameInfo>::iterator;
    using Compare = bool(const GameInfo &, const GameInfo &);

    if (file_sort == SortBy::name) {
      // Sort by filename
      insertion_sort<Iterator, Compare>(game_list.begin(), game_list.end(), [](const auto &a, const auto &b) { return a.title < b.title; });
    }

    if (file_sort == SortBy::size) {
      // Sort by filesize
      insertion_sort<Iterator, Compare>(game_list.begin(), game_list.end(), [](const auto &a, const auto &b) { return a.size < b.size; });
    }
}

void load_directory_list(std::string directory) {
  directory_list.clear();

  for(auto &folder : ::list_files(directory)) {
    if(folder.flags & blit::FileFlags::directory) {
      if(folder.name.compare("System Volume Information") == 0 || folder.name[0] == '.') continue;
      directory_list.push_back({folder.name, 0, 0});
    }
  }

  directory_list.sort([](const auto &a, const auto &b) { return a.name > b.name; });

  directory_list.push_front({"/", 0, 0});
  directory_list.push_front({"flash:", 0, 0});

  // measure positions
  int x = 0;
  for(auto &dir : directory_list) {
    dir.x = x;
    dir.w = screen.measure_text(dir.name == "/" ? "ROOT" : dir.name, minimal_font).w;

    x += dir.w + 10;
  }
}

bool parse_file_metadata(const std::string &filename, BlitGameMetadata &metadata, bool unpack_images = false) {
  blit::File f(filename);
  uint32_t offset = 0;

  BlitGameHeader header;
  auto read = f.read(offset, sizeof(header), (char *)&header);

  // skip relocation data
  if(header.magic == 0x4F4C4552 /* RELO */) {
    uint32_t num_relocs;
    f.read(4, 4, (char *)&num_relocs);

    offset = num_relocs * 4 + 8;
    // re-read header
    f.read(offset, sizeof(header), (char *)&header);
  }

  if(header.magic == blit_game_magic) {
    uint8_t buf[10];

    offset += (header.end & 0x1FFFFFF);
    auto bytes_read = f.read(offset, 10, (char *)buf);

    if(bytes_read == 10 && memcmp(buf, "BLITMETA", 8) == 0) {
      // don't bother reading the whole thing if we don't want the images
      auto metadata_len = unpack_images ? *reinterpret_cast<uint16_t *>(buf + 8) : sizeof(RawMetadata);

      uint8_t metadata_buf[0xFFFF];
      f.read(offset + 10, metadata_len, (char *)metadata_buf);

      parse_metadata(reinterpret_cast<char *>(metadata_buf), metadata_len, metadata, unpack_images);

      return true;
    }
  }

  return false;
}

void load_file_list(std::string directory) {

  game_list.clear();

  for(auto &file : ::list_files(directory)) {
    if(file.flags & blit::FileFlags::directory)
      continue;

    if(file.name.length() < 6) // minimum length for single-letter game (a.blit)
      continue;

    if (file.name[0] == '.') // hidden file
      continue;

    if(file.name.compare(file.name.length() - 5, 5, ".blit") == 0 || file.name.compare(file.name.length() - 5, 5, ".BLIT") == 0) {

      GameInfo game;
      game.title = file.name.substr(0, file.name.length() - 5);
      game.filename = directory == "/" ? file.name : directory + "/" + file.name;
      game.size = file.size;
      
      // check for metadata
      BlitGameMetadata meta;
      if(parse_file_metadata(game.filename, meta)) {
        game.title = meta.title;
        game.checksum = meta.crc32;
      }

      game_list.push_back(game);
    }
  }

  auto total_items = game_list.size();
  if(persist.selected_menu_item >= total_items)
    persist.selected_menu_item = total_items - 1;

  sort_file_list();
}

void load_current_game_metadata() {
  bool loaded = false;

  if(!game_list.empty()) {
    auto &game = game_list[persist.selected_menu_item];

    loaded = parse_file_metadata(game.filename, selected_game_metadata, true);
  }

  // no valid metadata, reset
  if(!loaded) {
    selected_game_metadata.free_surfaces();
    selected_game_metadata = BlitGameMetadata();
  }
}

bool launch_game_from_sd(const char *path) {
  return api.launch(path);
}

void init_lists() {
  load_directory_list("/");
  current_directory = directory_list.begin();

  load_file_list(current_directory->name);

  load_current_game_metadata();
}

void scan_flash() {
#ifdef TARGET_32BLIT_HW
  const uint32_t qspi_flash_sector_size = 64 * 1024;
  const uint32_t qspi_flash_size = 32768 * 1024;
  const uint32_t qspi_flash_address = 0x90000000;

  for(uint32_t offset = 0; offset < qspi_flash_size;) {
    auto header = *(BlitGameHeader *)(qspi_flash_address + offset);

    if(header.magic != blit_game_magic) {
      offset += qspi_flash_sector_size;
      continue;
    }

    uint32_t size = header.end - qspi_flash_address;

    // tiny bit of metadata parsing just to get the size
    auto buf = (char *)(qspi_flash_address + offset + size);
    if(memcmp(buf, "BLITMETA", 8) == 0)
      size += *(uint16_t *)(buf + 8) + 10;

    File::add_buffer_file("flash:/" + std::to_string(offset / qspi_flash_sector_size) + ".blit", (uint8_t *)(qspi_flash_address + offset), size);

    offset += calc_num_blocks(size) * qspi_flash_sector_size;
  }
#endif
}

void init() {
  set_screen_mode(ScreenMode::hires);
  screen.clear();

  spritesheet = SpriteSheet::load(sprites);

  scan_flash();
  init_lists();

  // error reset handling
  /*if(persist.reset_error) {
    dialog.show("Oops!", "Restart game?", [](bool yes){

      if(yes)
        launch_game(persist.last_game_offset);

      persist.reset_error = false;
    });
  }*/
}

void render(uint32_t time) {
  screen.sprites = spritesheet;

  screen.pen = Pen(5, 8, 12);
  screen.clear();

  screen.pen = Pen(0, 0, 0, 100);
  screen.rectangle(Rect(10, 0, 100, 240));

  // adjust alignment rect for vertical spacing
  const int text_align_height = ROW_HEIGHT + minimal_font.spacing_y;

  // list folders
  if(!directory_list.empty()) {
    screen.clip = Rect(120, 5, 190, text_align_height);

    for(auto &directory : directory_list) {
      if(directory.name == current_directory->name)
        screen.pen = Pen(235, 245, 255);
      else
        screen.pen = Pen(80, 100, 120);

      int x = 120 + 95 + directory.x - directory_list_scroll_offset;
      screen.text(directory.name == "/" ? "ROOT" : directory.name, minimal_font, Rect(x, 5, 190, text_align_height), true, TextAlign::center_v);
    }

    screen.clip = Rect(Point(0, 0), screen.bounds);
  }

  int y = 115 - file_list_scroll_offset.y;
  uint32_t i = 0;

  // list games
  if(!game_list.empty()) {
    const int size_x = 115;

    screen.clip = Rect(10, 0, 90, 240);
    for(auto &file : game_list) {
      if(i++ == persist.selected_menu_item)
        screen.pen = Pen(235, 245, 255);
      else
        screen.pen = Pen(80, 100, 120);

      screen.text(file.title, minimal_font, Rect(file_list_scroll_offset.x, y, 100 - 20, text_align_height), true, TextAlign::center_v);
      y += ROW_HEIGHT;
    }
    screen.clip = Rect(Point(0, 0), screen.bounds);

    // action icons
    // delete
    screen.sprite(2, Point(120, 20));
    screen.sprite(0, Point(130, 20));

    // run
    screen.sprite(1, Point(120, 32));
    screen.sprite(0, Point(130, 32), SpriteTransform::R90);

    // game info
    if(selected_game_metadata.splash)
      screen.blit(selected_game_metadata.splash, Rect(Point(0, 0), selected_game_metadata.splash->bounds), Point(172, 20));

    screen.pen = Pen(235, 245, 255);
    screen.text(selected_game_metadata.title, minimal_font, Point(172, 124));

    Rect desc_rect(172, 138, 128, 64);

    screen.pen = Pen(80, 100, 120);
    std::string wrapped_desc = screen.wrap_text(selected_game_metadata.description, desc_rect.w, minimal_font);
    screen.text(wrapped_desc, minimal_font, desc_rect);

    screen.text(selected_game_metadata.author, minimal_font, Point(172, 200));
    screen.text(selected_game_metadata.version, minimal_font, Point(172, 212));

    int num_blocks = calc_num_blocks(game_list[persist.selected_menu_item].size);
    char buf[20];
    snprintf(buf, 20, "%i block%s", num_blocks, num_blocks == 1 ? "" : "s");
    screen.text(buf, minimal_font, Point(172, 224));
  }
  else {
    screen.pen = Pen(235, 245, 255);

    if(/*current_directory->name != "FLASH" &&*/ !blit::is_storage_available())
      screen.text("No SD Card\nDetected.", minimal_font, Point(60, screen.bounds.h / 2), true, TextAlign::center_center);
    else
      screen.text("No Games Found.", minimal_font, Point(60, screen.bounds.h / 2), true, TextAlign::center_center);
  }

  //progress.draw();
  dialog.draw();
}

void update(uint32_t time) {

  if(blit::is_storage_available() != sd_detected) {
    init_lists();
    sd_detected = blit::is_storage_available();
  }

  if(dialog.update())
    return;

  bool button_a = buttons.released & Button::A;
  bool button_x = buttons.pressed & Button::X;
  bool button_y = buttons.pressed & Button::Y;

  static uint16_t button_repeat = 150;
  static uint32_t last_repeat = 0;
  static uint32_t hold_time = 0;

  static bool last_button_up = false;
  static bool last_button_down = false;
  static bool last_button_left = false;
  static bool last_button_right = false;

  bool current_button_up = (buttons.state & Button::DPAD_UP) || joystick.y < -0.2f;
  bool current_button_down = (buttons.state & Button::DPAD_DOWN) || joystick.y > 0.2f;
  bool current_button_left = (buttons.state & Button::DPAD_LEFT) || joystick.x < -0.5f;
  bool current_button_right = (buttons.state & Button::DPAD_RIGHT) || joystick.x > 0.5f;

  bool button_up = current_button_up & !last_button_up;
  bool button_down = current_button_down & !last_button_down;
  bool button_left  = current_button_left & !last_button_left;
  bool button_right = current_button_right & !last_button_right;

  last_button_up = current_button_up;
  last_button_down = current_button_down;

  // Tight ramping auto-repeat for up/down on joystick or d-pad
  if (current_button_up || current_button_down) {
    hold_time++;
    // if(button_repeat > 10) button_repeat--; // Ramping
    if(hold_time > 50) button_repeat = 10; // Jump to fast mode when held
    if(hold_time - last_repeat > button_repeat) {
      last_button_up = false;
      last_button_down = false;
      last_repeat = hold_time;
    }
  } else {
    hold_time = 0;
    last_repeat = 0;
    button_repeat = 150;
  }

  last_button_left = current_button_left;
  last_button_right = current_button_right;


  auto total_items = game_list.size();

  auto old_menu_item = persist.selected_menu_item;

  if(button_up)
  {
    if(persist.selected_menu_item > 0) {
      persist.selected_menu_item--;
    } else {
      persist.selected_menu_item = total_items - 1;
    }
  }

  if(button_down)
  {
    if(persist.selected_menu_item < (total_items - 1)) {
      persist.selected_menu_item++;
    } else {
      persist.selected_menu_item = 0;
    }
  }

  // switch between flash and SD lists
  if(button_left) {
    if(current_directory == directory_list.begin())
      current_directory = --directory_list.end();
    else
      --current_directory;
  }

  if(button_right) {
    current_directory++;
    if(current_directory == directory_list.end()) {
      current_directory = directory_list.begin();
    }
  }

  if(button_left || button_right) {
    load_file_list(current_directory->name);

    persist.selected_menu_item = 0;
    load_current_game_metadata();
  }

  // scroll list towards selected item  
  file_list_scroll_offset.y += ((persist.selected_menu_item * 10) - file_list_scroll_offset.y) / 5.0f;

  directory_list_scroll_offset += (current_directory->x + current_directory->w / 2 - directory_list_scroll_offset) / 5.0f;

  // load metadata for selected item
  if(persist.selected_menu_item != old_menu_item)
    load_current_game_metadata();

  if(button_a && !game_list.empty())
  {
    uint32_t offset = 0xFFFFFFFF;
    auto game = game_list[persist.selected_menu_item];

    launch_game_from_sd(game.filename.c_str());
  }

  // delete current game
  if (button_x && !game_list.empty()) {
    auto &game = game_list[persist.selected_menu_item];

    dialog.show("Confirm", "Really delete " + game.title + "?", [](bool yes){
      if(yes) {
        auto &game = game_list[persist.selected_menu_item];
        if(game.filename.compare(0, 7, "flash:/") == 0)
          api.erase_game(std::stoi(game.filename.substr(7)) * qspi_flash_sector_size);
        
        ::remove_file(game.filename);

        load_file_list(current_directory->name);
        load_current_game_metadata();
      }
    });
  }

  if (button_y) {
    file_sort = file_sort == SortBy::name ? SortBy::size : SortBy::name;
    sort_file_list();
  }
}

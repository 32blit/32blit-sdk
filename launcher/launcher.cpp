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

#include "theme.hpp"

#include "credits.hpp"

Dialog dialog;

using namespace blit;

const Font launcher_font(font8x8);

constexpr uint32_t qspi_flash_sector_size = 64 * 1024;

static Screen currentScreen = Screen::main;

bool show_fps = false;
bool sd_detected = true;
Vec2 file_list_scroll_offset(10.0f, 0.0f);
Point game_info_offset(120, 20);
Point game_actions_offset(game_info_offset.x + 128 + 8, 28);
float directory_list_scroll_offset = 0.0f;

std::vector<GameInfo> game_list;
std::list<DirectoryInfo> directory_list;
std::list<DirectoryInfo>::iterator current_directory;

SortBy file_sort = SortBy::name;

GameInfo selected_game;
BlitGameMetadata selected_game_metadata;

Surface *spritesheet;
Surface *screenshot;

AutoRepeat ar_button_up(250, 600);
AutoRepeat ar_button_down(250, 600);
AutoRepeat ar_button_left(0, 0);
AutoRepeat ar_button_right(0, 0);

uint8_t screenshot_buf[320 * 240 * 3];

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

  auto dir_filter = [](const FileInfo &info){
    if(!(info.flags & FileFlags::directory))
      return false;

    if(info.name.compare("System Volume Information") == 0 || info.name[0] == '.')
      return false;

    return true;
  };

  for(auto &folder : ::list_files(directory, dir_filter))
    directory_list.push_back({folder.name, 0, 0});

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

  if(!f.is_open())
    return false;

  uint32_t offset = 0;

  uint8_t buf[sizeof(BlitGameHeader)];
  auto read = f.read(offset, sizeof(buf), (char *)&buf);

  // skip relocation data
  if(memcmp(buf, "RELO", 4) == 0) {
    uint32_t num_relocs;
    f.read(4, 4, (char *)&num_relocs);

    offset = num_relocs * 4 + 8;
    // re-read header
    read = f.read(offset, sizeof(buf), (char *)&buf);
  }

  // game header - skip to metadata
  if(memcmp(buf, "BLITMETA", 8) != 0) {
    auto &header = *(BlitGameHeader *)buf;
    if(read == sizeof(BlitGameHeader) && header.magic == blit_game_magic) {
      offset += (header.end & 0x1FFFFFF);
      read = f.read(offset, 10, (char *)buf);
    }
  }

  if(read >= 10 && memcmp(buf, "BLITMETA", 8) == 0) {
    // don't bother reading the whole thing if we don't want the images
    auto metadata_len = unpack_images ? *reinterpret_cast<uint16_t *>(buf + 8) : sizeof(RawMetadata);

    uint8_t metadata_buf[0xFFFF];
    f.read(offset + 10, metadata_len, (char *)metadata_buf);

    parse_metadata(reinterpret_cast<char *>(metadata_buf), metadata_len, metadata, unpack_images);

    return true;
  }

  return false;
}

void load_file_list(std::string directory) {

  game_list.clear();

  auto files = list_files(directory, [](auto &file) {
    if(file.flags & FileFlags::directory)
      return false;

    if(file.name.length() < 6) // minimum length for single-letter game (a.blit)
      return false;

    if(file.name[0] == '.') // hidden file
      return false;

    if(file.name.find_last_of('.') == std::string::npos) // no extension
      return false;

    return true;
  });

  game_list.reserve(files.size()); // worst case

  for(auto &file : files) {
    auto last_dot = file.name.find_last_of('.');

    auto ext = file.name.substr(file.name.find_last_of('.') + 1);

    for(auto &c : ext)
      c = tolower(c);

    if(ext == "blit") {

      GameInfo game;
      game.type = GameType::game;
      game.title = file.name.substr(0, file.name.length() - 5);
      game.filename = directory == "/" ? file.name : directory + "/" + file.name;
      game.size = file.size;

      // check for metadata
      BlitGameMetadata meta;
      if(parse_file_metadata(game.filename, meta)) {
        game.title = meta.title;
      }

      game_list.push_back(game);
      continue;
    }

    if(ext == "bmp" || ext == "blim") {
      GameInfo game;
      game.type = GameType::screenshot;
      game.title = file.name.substr(0, file.name.length() - ext.length() - 1);
      game.filename = directory == "/" ? file.name : directory + "/" + file.name;
      game.size = file.size;

      // Special case check for an installed handler for these types, ie: a sprite editor
      game.can_launch = api.get_type_handler_metadata && api.get_type_handler_metadata(ext.c_str());
      game_list.push_back(game);
      continue;
    }

    if(!api.get_type_handler_metadata) continue;

    auto handler_meta = api.get_type_handler_metadata(ext.c_str());

    if(handler_meta) {
      GameInfo game;
      game.type = GameType::file;
      game.filename = directory == "/" ? file.name : directory + "/" + file.name;
      strncpy(game.ext, ext.c_str(), 5);
      game.ext[4] = 0;
      game.size = file.size;
      game.can_launch = true;

      // check got a metadata file
      BlitGameMetadata meta;
      auto meta_filename = game.filename + ".blmeta";
      if(parse_file_metadata(meta_filename, meta))
        game.title = meta.title;
      else
        game.title = file.name;

      game_list.push_back(game);
    }
  }

  int total_items = (int)game_list.size();
  if(selected_menu_item >= total_items)
    selected_menu_item = total_items - 1;

  // probably doesn't do anything...
  game_list.shrink_to_fit();

  sort_file_list();
}

void load_current_game_metadata() {
  static std::string current_screenshot = "";
  bool loaded = false;

  if(!game_list.empty()) {
    selected_game = game_list[selected_menu_item];

    if(selected_game.type == GameType::file) {
      // not a .blit - look for a metadata file
      auto meta_filename = selected_game.filename + ".blmeta";
      if(!parse_file_metadata(meta_filename, selected_game_metadata, true)) {
        // fallback to handler metadata/placeholders
        auto handler_meta = (char *)api.get_type_handler_metadata(selected_game.ext);
        auto len = *reinterpret_cast<uint16_t *>(handler_meta + 8);
        parse_metadata(handler_meta + 10, len, selected_game_metadata, true);

        selected_game_metadata.description = "Launches with: " + selected_game_metadata.title;
        selected_game_metadata.title = selected_game.title;
        selected_game_metadata.author = "";
        selected_game_metadata.version = "";
      }
      loaded = true;
    } else
      loaded = parse_file_metadata(selected_game.filename, selected_game_metadata, true);
  }

  if(selected_game.type == GameType::screenshot) {
    if(selected_game.filename != current_screenshot) {
      // Free any old buffers
      if(screenshot) {
        delete[] screenshot->palette;
        delete screenshot;
        screenshot = nullptr;
      }
      // Load the new screenshot
      screenshot = Surface::load(selected_game.filename, screenshot_buf, sizeof(screenshot_buf));
    }
  } else {
    // Not showing a screenshot, free the buffers
    if(screenshot) {
      delete[] screenshot->palette;
      delete screenshot;
      screenshot = nullptr;
    }
  }

  // no valid metadata, reset
  if(!loaded) {
    selected_game_metadata.free_surfaces();
    selected_game_metadata = BlitGameMetadata();
  }
}

bool launch_current_game() {
  return api.launch(selected_game.filename.c_str());
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

void delete_current_game() {
  dialog.show("Confirm", "Really delete " + selected_game.title + "?", [](bool yes){
    if(yes) {
      if(selected_game.filename.compare(0, 7, "flash:/") == 0)
        api.erase_game(std::stoi(selected_game.filename.substr(7)) * qspi_flash_sector_size);

      ::remove_file(selected_game.filename);

      load_file_list(current_directory->name);
      load_current_game_metadata();
    }
  });
}

void init() {
  set_screen_mode(ScreenMode::hires);
  screen.clear();

  selected_menu_item = 0;

  init_theme();

  spritesheet = Surface::load(sprites);

  scan_flash();
  init_lists();

  credits::prepare();
}

void swoosh(uint32_t time, float t1, float t2, float s1, float s2, int t0, int offset_y=120, int size=60, int alpha=64) {
  constexpr int swoosh_resolution = 32;
  for(auto x = 0; x < screen.bounds.w / swoosh_resolution; x++) {
    float t_a = (x / s1) + float(time + t0) / t1;
    float t_b = (x / s2) + float(time + t0) / t2;

    int y1 = sinf(t_a) * size;
    int y2 = sinf(t_b) * size;

    if(y1 > y2) std::swap(y1, y2);

    y1 += offset_y;
    y2 += offset_y + 2;

    int range = y2 - y1;

    for(auto y = 0; y <= range; y++) {
      if(y > range / 2){
        screen.pen.a = alpha - (alpha * y / range);
      }
      else
      {
        screen.pen.a = alpha * y / range;
      }
      // This is an optimisation, not an aesthetic choice!
      screen.h_span(Point(x * swoosh_resolution,  y1 + y), swoosh_resolution);
    }
  }
}

void render_fps(uint32_t us_start) {
  if(!show_fps) return;
  // draw FPS meter
  uint32_t us_end = now_us();
  uint32_t us_elapsed = us_diff(us_start, us_end);
  screen.mask = nullptr;

  screen.pen = Pen(0, 0, 0);
  screen.rectangle(Rect(Point(0, screen.bounds.h - 14), Size(game_info_offset.x - 10, 14)));

  screen.pen = Pen(255, 0, 0);
  for (unsigned int i = 0; i < us_elapsed / 1000; i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(Rect(i * 3 + 1, screen.bounds.h - 3, 2, 2));
  }

  screen.pen = Pen(255, 255, 255);
  screen.text(std::to_string(us_elapsed), minimal_font, Point(0, screen.bounds.h - 12));
}

void render(uint32_t time) {
  uint32_t us_start = now_us();
  screen.sprites = spritesheet;

  screen.pen = theme.color_background;
  screen.clear();

  if(currentScreen != Screen::screenshot && currentScreen != Screen::credits && selected_game.type != GameType::screenshot) {
    screen.pen = Pen(255, 255, 255);
    swoosh(time, 5100.0f, 3900.0f, 1900.0f, 900.0f, 3500);
    screen.pen = theme.color_accent;
    swoosh(time, 5000.0f, 3000.0f, 1000.0f, 1000.0f, 0);
    screen.pen = Pen(~theme.color_accent.r, ~theme.color_accent.g, ~theme.color_accent.b);
    swoosh(time, 5100.0f, 3900.0f, 900.0f, 1100.0f, 5000);
  }

  if(!game_list.empty() && selected_game.type == GameType::screenshot && screenshot) {
    if(screenshot->bounds.w == screen.bounds.w) {
      screen.blit(screenshot, Rect(Point(0, 0), screenshot->bounds), Point(0, 0));
    } else if(screenshot->bounds == Size(128, 128)) {
      screen.pen = Pen(0, 0, 0, 255);
      screen.rectangle(Rect(game_info_offset, Size(128, 128)));
      screen.blit(screenshot, Rect(Point(0, 0), screenshot->bounds), game_info_offset);
    } else {
      screen.stretch_blit(screenshot, Rect(Point(0, 0), screenshot->bounds), Rect(Point(0, 0), screen.bounds));
    }

    if(currentScreen == Screen::screenshot) {
      // back
      screen.sprite(5, Point(game_actions_offset.x, game_actions_offset.y + 12));
      screen.sprite(0, Point(game_actions_offset.x + 10, game_actions_offset.y + 12), SpriteTransform::R180);
      render_fps(us_start);
      return;
    }

    // Darken behind the file/directory menus so they're visible
    screen.pen = theme.color_background;
    screen.pen.a = 150;
    screen.rectangle(Rect(game_info_offset.x - 10, 0, screen.bounds.w - game_info_offset.x + 10, 20));
    screen.rectangle(Rect(0, 0, game_info_offset.x - 10, screen.bounds.h));
  }

  // adjust alignment rect for vertical spacing
  const int text_align_height = ROW_HEIGHT + minimal_font.spacing_y;

  // list folders
  if(!directory_list.empty()) {
    screen.clip = Rect(120, 5, 190, text_align_height);

    for(auto &directory : directory_list) {
      if(directory.name == current_directory->name)
        screen.pen = theme.color_accent;
      else
        screen.pen = theme.color_text;

      int x = 120 + 95 + directory.x - directory_list_scroll_offset;
      screen.text(directory.name == "/" ? "ROOT" : directory.name, minimal_font, Rect(x, 5, 190, text_align_height), true, TextAlign::center_v);
    }

    screen.clip = Rect(Point(0, 0), screen.bounds);
  }

  int y = 115 - file_list_scroll_offset.y;
  int i = 0;

  // list games
  if(!game_list.empty()) {
    screen.pen = theme.color_overlay;
    screen.rectangle(Rect(0, 0, game_info_offset.x - 10, screen.bounds.h));

    screen.clip = Rect(0, 0, game_info_offset.x - 20, screen.bounds.h);
    for(auto &file : game_list) {
      if(i++ == selected_menu_item)
        screen.pen = theme.color_accent;
      else
        screen.pen = theme.color_text;

      screen.text(file.title, launcher_font, Rect(file_list_scroll_offset.x, y, 90, text_align_height), true, TextAlign::center_v);
      y += ROW_HEIGHT;
    }
    screen.clip = Rect(Point(0, 0), screen.bounds);

    // delete
    screen.sprite(2, Point(game_actions_offset.x, game_actions_offset.y));
    screen.sprite(0, Point(game_actions_offset.x + 10, game_actions_offset.y));

    if(selected_game.type == GameType::screenshot) {
      if(screenshot->bounds == Size(128, 128)) {
        if(selected_game.can_launch){
          // edit (in sprite editor, presumably)
          screen.sprite(1, Point(game_actions_offset.x, game_actions_offset.y + 12));
          screen.sprite(0, Point(game_actions_offset.x + 10, game_actions_offset.y + 12), SpriteTransform::R90);
        }
      }
      else
      {
        // view screenshot fullscreen
        screen.sprite(4, Point(game_actions_offset.x, game_actions_offset.y + 12));
        screen.sprite(0, Point(game_actions_offset.x + 10, game_actions_offset.y + 12), SpriteTransform::R90);
      }
    } else {
      // run game / launch file
      screen.sprite(1, Point(game_actions_offset.x, game_actions_offset.y + 12));
      screen.sprite(0, Point(game_actions_offset.x + 10, game_actions_offset.y + 12), SpriteTransform::R90);

      // game info
      if(selected_game_metadata.splash)
        screen.blit(selected_game_metadata.splash, Rect(Point(0, 0), selected_game_metadata.splash->bounds), game_info_offset);

      screen.pen = theme.color_accent;
      std::string wrapped_title = screen.wrap_text(selected_game_metadata.title, screen.bounds.w - game_info_offset.x - 10, minimal_font);

      Size title_size = screen.measure_text(wrapped_title, launcher_font);
      screen.text(wrapped_title, launcher_font, Point(game_info_offset.x, game_info_offset.y + 104));

      Rect desc_rect(game_info_offset.x, game_info_offset.y + 108 + title_size.h, screen.bounds.w - game_info_offset.x - 10, 64);

      screen.pen = theme.color_text;
      std::string wrapped_desc = screen.wrap_text(selected_game_metadata.description, desc_rect.w, launcher_font);
      screen.text(wrapped_desc, launcher_font, desc_rect);

      screen.text(selected_game_metadata.author, minimal_font, Point(game_info_offset.x, screen.bounds.h - 32));
      screen.text(selected_game_metadata.version, minimal_font, Point(game_info_offset.x, screen.bounds.h - 24));

      int num_blocks = calc_num_blocks(selected_game.size);
      char buf[20];
      snprintf(buf, 20, "%i block%s", num_blocks, num_blocks == 1 ? "" : "s");
      screen.text(buf, minimal_font, Point(game_info_offset.x, screen.bounds.h - 16));
    }
  }
  else {
    screen.pen = theme.color_text;

    if(/*current_directory->name != "FLASH" &&*/ !blit::is_storage_available())
      screen.text("No SD Card\nDetected.", minimal_font, Point(screen.bounds.w / 2, screen.bounds.h / 2), true, TextAlign::center_center);
    else
      screen.text("No Games Found.", minimal_font, Point(screen.bounds.w / 2, screen.bounds.h / 2), true, TextAlign::center_center);
  }

  if (currentScreen == Screen::credits) {
    credits::render();
  }

  //progress.draw();
  dialog.draw();
  render_fps(us_start);
}

void update(uint32_t time) {

  if(blit::is_storage_available() != sd_detected) {
    init_lists();
    sd_detected = blit::is_storage_available();
  }

  bool button_a = buttons.released & Button::A;
  bool button_b = buttons.pressed & Button::B;
  bool button_x = buttons.pressed & Button::X;
  bool button_y = buttons.pressed & Button::Y;
  bool button_menu = buttons.pressed & Button::MENU;
  bool button_up = ar_button_up.next(time, buttons.state & Button::DPAD_UP || joystick.y < -0.2f);
  bool button_down = ar_button_down.next(time, buttons.state & Button::DPAD_DOWN || joystick.y > 0.2f);
  bool button_left = ar_button_left.next(time, buttons.state & Button::DPAD_LEFT || joystick.x < -0.5f);
  bool button_right = ar_button_right.next(time, buttons.state & Button::DPAD_RIGHT || joystick.x > 0.5f);

  if (currentScreen == Screen::credits) {
    credits::update(time);

    if (button_menu) {
      currentScreen = Screen::main;
    }

    if(button_y) {
      show_fps = !show_fps;
    }

    return;
  }

  if (button_menu) {
    credits::reset_scrolling();
    currentScreen = Screen::credits;
  }

  if(dialog.update())
    return;

  int total_items = (int)game_list.size();

  auto old_menu_item = selected_menu_item;

  if(button_up)
  {
    selected_menu_item--;
    if(selected_menu_item < 0) {
      selected_menu_item = total_items - 1;
    }
  }

  if(button_down)
  {
    selected_menu_item++;
    if(selected_menu_item > total_items - 1) {
      selected_menu_item = 0;
    }
  }

  if(currentScreen == Screen::screenshot) {
    // b to exit full screen screenshot view
    if(button_b) {
      currentScreen = Screen::main;
    }
  } else {
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

      selected_menu_item = 0;
      old_menu_item = -1;
    }

    if (button_y) {
      file_sort = file_sort == SortBy::name ? SortBy::size : SortBy::name;
      sort_file_list();
    }
  }

  // scroll list towards selected item
  file_list_scroll_offset.y += ((selected_menu_item * ROW_HEIGHT) - file_list_scroll_offset.y) / 5.0f;

  directory_list_scroll_offset += (current_directory->x + current_directory->w / 2 - directory_list_scroll_offset) / 5.0f;

  // load metadata for selected item
  if(selected_menu_item != old_menu_item) {
    load_current_game_metadata();
  }

  // paranoid bail out if you're browsing screenshots full screen and come across a game
  if(selected_game.type != GameType::screenshot && currentScreen == Screen::screenshot) {
    currentScreen = Screen::main;
  }

  // delete current game / screenshot
  if (button_x && !game_list.empty()) {
    delete_current_game();
  }

  if(!game_list.empty()) {
    if(button_a)
    {
      if(selected_game.type == GameType::screenshot && !selected_game.can_launch) {
        currentScreen = Screen::screenshot;
      }
      else {
        launch_current_game();
      }
    }
  }
}

#include <cstring>

#include "32blit.hpp"
#include "engine/api_private.hpp"

#include "executable.hpp"

using namespace blit;

const char *get_category(const uint8_t *ptr) {
  // check headers
  auto header = (const BlitGameHeader *)ptr;

  if(header->magic != blit_game_magic)
    return nullptr;

  auto metadata_ptr = ptr + header->end;

  if(memcmp(metadata_ptr, "BLITMETA", 8) != 0)
    return nullptr;

  // skip straight to the type block
  metadata_ptr += 10 + sizeof(RawMetadata);

  if(memcmp(metadata_ptr, "BLITTYPE", 8) != 0)
    return nullptr;

  // get the category from the type block
  auto type_meta = (const RawTypeMetadata *)(metadata_ptr + 8);

  return type_meta->category;
}

static bool try_launch(bool launcher) {
  bool done = false;

  api.list_installed_games([&done, launcher](const uint8_t *ptr, uint32_t block, uint32_t size){
    if(!done) {

      if(launcher) {
        // check category
        auto category = get_category(ptr);

        if(!category || strcmp(category, "launcher") != 0)
          return;
      }

      auto path = "flash:/" + std::to_string(block) + ".blit";
      done = api.launch(path.c_str());
    }
  });

  return done;
}

void init() {
  bool done = false;

  // try to find and launch a launcher
  done = try_launch(true);

  if(!done) {
    // fall back to launcher in storage
    // TODO: auto-update
    if(file_exists("launcher.blit")) {
      done = api.launch("launcher.blit");
    }
  }

  if(!done) {
    // as a last resort launch the first thing we find
    try_launch(false);
  }
}

void render(uint32_t time) {
}

void update(uint32_t time) {
}

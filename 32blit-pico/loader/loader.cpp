#include "32blit.hpp"
#include "engine/api_private.hpp"

using namespace blit;

void init() {
  bool done = false;
  // launch the first thing we find
  // TODO: find launcher
  api.list_installed_games([&done](const uint8_t *ptr, uint32_t block, uint32_t size){
    if(!done) {
      auto path = "flash:/" + std::to_string(block) + ".blit";
      done = api.launch(path.c_str());
    }
  });
}

void render(uint32_t time) {
}

void update(uint32_t time) {
}

#pragma once

namespace blit {
  const char *get_version_string();
  const char *get_build_date();

  struct GameMetadata {
    const char *title = nullptr;
    const char *author = nullptr;
    const char *description = nullptr;
    const char *version = nullptr;
    const char *url = nullptr;
    const char *category = nullptr;
  };

  GameMetadata get_metadata();
}

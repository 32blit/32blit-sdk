#include "version.hpp"
#include "version_defs.hpp"
#include "api_private.hpp"

namespace blit {
  const char *get_version_string() {
    return BLIT_BUILD_VER;
  }
  const char *get_build_date() {
    return BLIT_BUILD_DATE;
  }

  GameMetadata get_metadata() {
    return api.get_metadata();
  }
}

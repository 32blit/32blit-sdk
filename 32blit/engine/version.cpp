#include "version.hpp"
#include "version_defs.hpp"

namespace blit {
  const char *get_version_string() {
    return BLIT_BUILD_VER;
  }
  const char *get_build_date() {
    return BLIT_BUILD_DATE;
  }
}
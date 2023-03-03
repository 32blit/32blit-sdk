#include "engine/api_private.hpp"

extern char __api_start;
#pragma GCC diagnostic ignored "-Warray-bounds"

namespace blit {
  // for compatibility reasons, these are the same thing
  const APIConst &api = *(APIConst *)&__api_start;
  APIData &api_data = *(APIData *)&__api_start;
}

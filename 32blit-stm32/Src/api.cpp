#include "engine/api_private.hpp"

extern char __api_start;
#pragma GCC diagnostic ignored "-Warray-bounds"

namespace blit {
  API &api = *(API *)&__api_start;
}

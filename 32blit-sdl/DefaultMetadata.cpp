// Default metadata if there is non compiled into the game
// There should be nothing else in this file so that the VS linker drops it when not needed
#ifdef _MSC_VER
#define WEAK
#else
#define WEAK [[gnu::weak]]
#endif

WEAK const char *metadata_title = "32Blit Game";
WEAK const char *metadata_author = "Unknown";
WEAK const char *metadata_description = "";
WEAK const char *metadata_version = "v0.0.0";
WEAK const char *metadata_url = "";
WEAK const char *metadata_category = "";

#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

#include "ff.h"

#include "file.hpp"
#include "32blit.h"
#include "executable.hpp"
#include "USBManager.h"

extern USBManager g_usbManager;

bool is_filesystem_access_disabled() {
  return g_usbManager.GetType() == USBManager::usbtMSC;
}

static char save_path[32]; // max game title length is 24 + ".blit/" + "/"

const char *get_save_path() {
  const char *app_name;
  char buf[10];

  if(!directory_exists(".blit"))
    create_directory(".blit");

  if(!blit_user_code_running())
    app_name = "_firmware";
  else {
    auto meta = blit_get_running_game_metadata();

    if(meta)
      app_name = meta->title;
    else {
      // fallback to offset
      snprintf(buf, 10, "%li", persist.last_game_offset);
      app_name = buf;
    }
  }

  snprintf(save_path, sizeof(save_path), ".blit/%s/", app_name);

  // make sure it exists
  if(!directory_exists(save_path))
    create_directory(save_path);

  return save_path;
}

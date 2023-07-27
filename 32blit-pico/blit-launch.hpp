#pragma once

#include <functional>

#include "executable.hpp"
#include "engine/api_private.hpp"

RawMetadata *get_running_game_metadata();

bool launch_file(const char *path);
blit::CanLaunchResult can_launch(const char *path);
void delayed_launch();

void list_installed_games(std::function<void(const uint8_t *, uint32_t, uint32_t)> callback);
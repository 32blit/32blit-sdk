#pragma once
#include <string>

void init();
void update(uint32_t time);
void render(uint32_t time);

extern const char *metadata_title;
extern const char *metadata_author;
extern const char *metadata_description;
extern const char *metadata_version;
extern const char *metadata_url;
extern const char *metadata_category;

/* Added a command line ability to specify a launch_path parameter. */
extern const char *launch_path;
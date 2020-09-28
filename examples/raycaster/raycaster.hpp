#pragma once

#include <cstdint>

#include "32blit.hpp"

constexpr float M_PI_H = 1.5707963267948966f;

constexpr float EPSILON = 0.00000001f;

constexpr uint16_t SCREEN_WIDTH = 160;
constexpr uint16_t SCREEN_HEIGHT = 120;
constexpr uint16_t VIEW_HEIGHT = (SCREEN_HEIGHT - 24);
constexpr uint16_t HORIZON = (VIEW_HEIGHT / 2);
constexpr uint8_t TEXTURE_WIDTH = 8;
constexpr uint8_t TEXTURE_HEIGHT = 8;
constexpr uint8_t TEXTURE_SCALE = 1;
constexpr uint8_t PLAYER_FOV = 90;
constexpr float HALF_FOV = PLAYER_FOV / 360.0f * blit::pi;

constexpr uint8_t MAP_WIDTH = 16;
constexpr uint8_t MAP_HEIGHT = 16;
constexpr uint8_t MAP_TILE_SOLID = 0x80;
constexpr uint8_t MAP_TILE_GRAFITTI = 0x40;
constexpr uint8_t MAX_RAY_STEPS = 24; //11 //sqrt((MAP_WIDTH ** 2) + (MAP_HEIGHT ** 2))

constexpr uint8_t TEXTURE_FLOOR = 0;
constexpr uint8_t TEXTURE_CEILING = 1;
constexpr uint8_t TEXTURE_WALL = 2;

constexpr float SPRITE_SCALE = 1.6f;


blit::Vec2 rotate_point(blit::Vec2 p, blit::Vec2 v);
blit::Vec2 rotate_vector(blit::Vec2 v, float a);

//void cast_floor();
void render_world(uint32_t time);
void render_sprites(uint32_t time);
void render_sky();
void render_stars();

struct player {
	blit::Vec2 direction;
	blit::Vec2 position;
	blit::Vec2 camera;
	float half_fov;
};

struct sprite {
	blit::Vec2 position;
	uint8_t texture;
	uint8_t color;
	float distance;
	const bool operator < (const sprite& rhs) const { return distance > rhs.distance; };
};

struct star {
	blit::Point position;
	uint8_t brightness;
};

extern blit::SpriteSheet sprites;
extern blit::Timer timer1;

void edges();
void blur(uint8_t passes);

void init();
void update(uint32_t time);
void render(uint32_t time);

void update_player_camera_plane();

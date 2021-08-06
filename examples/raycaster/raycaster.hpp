#pragma once

#include <cstdint>

#include "32blit.hpp"

constexpr float M_PI_H = 1.5707963267948966f;

constexpr float EPSILON = 0.00000001f;

#ifdef DISPLAY_ST7789
constexpr uint16_t OFFSET_TOP = 30;
constexpr uint16_t SCREEN_WIDTH = 120;
constexpr uint16_t SCREEN_HEIGHT = 120;
constexpr uint16_t NUM_SPRITES = 500;
constexpr uint16_t NUM_STARS = 50;
#define AMBIENT_OCCLUSION // Paints into a mask with a blur pass, disable for a slight performance boost
//#define SHOW_FPS          // Frame time is pretty high on PicoSystem, so just turn off the FPS meter :D
#else
constexpr uint16_t OFFSET_TOP = 0;
constexpr uint16_t SCREEN_WIDTH = 160;
constexpr uint16_t SCREEN_HEIGHT = 120;
constexpr uint16_t NUM_SPRITES = 5000;
constexpr uint16_t NUM_STARS = 100;
#define AMBIENT_OCCLUSION
#define SHOW_FPS
#endif

constexpr uint16_t VIEW_HEIGHT = SCREEN_HEIGHT - OFFSET_TOP - 24;
constexpr float SPRITE_SCALE = float(VIEW_HEIGHT) / 60.0f;

constexpr uint16_t HORIZON = VIEW_HEIGHT / 2;
constexpr uint8_t TEXTURE_WIDTH = 8;
constexpr uint8_t TEXTURE_HEIGHT = 8;
constexpr uint8_t TEXTURE_SCALE = 1;

constexpr uint8_t MAP_WIDTH = 16;
constexpr uint8_t MAP_HEIGHT = 16;
constexpr uint8_t MAX_RAY_STEPS = 24; //11 //sqrt((MAP_WIDTH ** 2) + (MAP_HEIGHT ** 2))


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
	float inverse_det;
	float half_fov;
	bool spraying;
	bool facing;
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

void blur(uint8_t passes);

void init();
void update(uint32_t time);
void render(uint32_t time);

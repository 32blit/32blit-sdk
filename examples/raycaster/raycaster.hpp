#pragma once

#include <cstdint>

#include "32blit.hpp"

constexpr float M_PI_H = 1.5707963267948966f;

constexpr float EPSILON = 0.00000001f;

#ifdef PICO_BOARD
constexpr uint16_t OFFSET_TOP = 30;
constexpr uint16_t SCREEN_WIDTH = 120;
constexpr uint16_t SCREEN_HEIGHT = 120;
constexpr uint16_t NUM_SPRITES = 1000;
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
//#define SHOW_FPS
#endif

constexpr uint8_t MAX_SPRAY = 100;  // Uses capacity from NUM_SPRITES
constexpr uint8_t MAX_ENEMIES = 10; // Uses capacity from NUM_SPRITES

constexpr float MAX_SPRAY_HEALTH = 128;

constexpr uint16_t VIEW_HEIGHT = SCREEN_HEIGHT - OFFSET_TOP - 24;
constexpr float SPRITE_SCALE = float(VIEW_HEIGHT) / 60.0f;

constexpr uint16_t HORIZON = VIEW_HEIGHT / 2;
constexpr uint8_t TEXTURE_WIDTH = 8;
constexpr uint8_t TEXTURE_HEIGHT = 8;
constexpr uint8_t TEXTURE_SCALE = 1;

constexpr uint8_t MAP_WIDTH = 16;
constexpr uint8_t MAP_HEIGHT = 16;
constexpr uint8_t MAX_RAY_STEPS = 24; //11 //sqrt((MAP_WIDTH ** 2) + (MAP_HEIGHT ** 2))

struct Entity {
	blit::Vec2 position;
	blit::Vec2 velocity;
	float rotation;
	float health;
};

struct Player : Entity {
	blit::Vec2 direction;
	blit::Vec2 camera;
	float inverse_det;
	bool spraying;
	bool facing;
};

enum SpriteType {
	PLANT = 0,
	WASP = 1,
	SPRAY = 2
};

enum SpriteTexture : uint8_t {
	FULL_TREE = 0,
	MATURE_TREE = 1,
	TALL_SHRUB = 2,
	SHORT_SHRUB = 3,
	TALL_GASS = 4,
	MID_GRASS = 5,
	SMOL_GRASS = 6,
	ANGRY_WASP = 7,
	BUGSPRAY = 8
};

struct Sprite : Entity {
	uint8_t size;
	SpriteTexture texture;
	SpriteType type;
	uint8_t color;
	float distance;
	bool active;
	bool operator < (const Sprite& rhs) const { return distance > rhs.distance; };
};

struct Star {
	blit::Point position;
	uint8_t brightness;
};

void init();
void update(uint32_t time);
void render(uint32_t time);

void render_world(uint32_t time);
void render_sprites(uint32_t time);
void render_spray(uint32_t time);
void render_sky();
void render_stars();

void blur(uint8_t passes);

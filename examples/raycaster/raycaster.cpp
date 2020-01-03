#include <string.h>
#include <string>
#include <sstream>
#include <cstdlib>
#include <array>
#include "raycaster.hpp"
#include "32blit.hpp"

using namespace blit;

const uint16_t screen_width = 160;
const uint16_t screen_height = 120;

uint8_t __ss[160 * 160] __SECTION__(".ss");
uint8_t __m[160 * 120] __SECTION__(".m");

float z_buffer[SCREEN_WIDTH];
float lut_camera_displacement[SCREEN_WIDTH];

const int num_sprites = 5000;
std::vector<sprite> map_sprites(num_sprites);

const int num_stars = 100;
std::vector<star> stars(num_stars);

/* create surfaces */
//surface ss((uint8_t *)__ss, size(160, 160), pixel_format::P);
surface m((uint8_t *)__m, pixel_format::M, size(screen_width, screen_height));

//spritesheet my_sprites(ss, 8, 8);


player player1{ vec2(0,0), vec2(0,0), vec2(0,0), 0 };
int bug_spraying = 0;

uint32_t tick_seed = 0;

float tan_half_fov;

bool flip_doom_guy = false;
point tile_in_view;
float grafitti_alpha = 0;

void blur(uint8_t passes);

Map map(rect(0, 0, 16, 16));
MapLayer *map_layer_walls;
MapLayer *map_layer_floor;

enum TileFlags { WALL = 1, NO_GRASS = 2 };

enum TileFacing { NONE = 0, NORTH = 1, SOUTH = 2, EAST = 4, WEST = 8 };

bool visibility_map[MAP_WIDTH * MAP_HEIGHT] = { 0 };

std::vector<uint8_t> map_data_walls = {
	0x01, 0x02, 0x03, 0x04, 0x03, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x01, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01,
	0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};

std::vector<uint8_t> map_data_floor = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x01, 0x05, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x05, 0x03, 0x04, 0x05, 0x01, 0x01, 0x05, 0x00, 0x01, 0x01, 0x01, 0x01, 0x05, 0x01, 0x00,
	0x00, 0x04, 0x01, 0x05, 0x01, 0x01, 0x05, 0x01, 0x00, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x02, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x03, 0x01, 0x00, 0x04, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x05, 0x01, 0x00, 0x01, 0x02, 0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x05, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00,
	0x00, 0x01, 0x01, 0x00, 0x05, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x04, 0x05, 0x01, 0x00,
	0x00, 0x05, 0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x00, 0x05, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00,
	0x00, 0x01, 0x05, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x05, 0x01, 0x00, 0x01, 0x00,
	0x00, 0x01, 0x03, 0x00, 0x03, 0x01, 0x05, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,
	0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00,
	0x00, 0x05, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x05, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x01, 0x01, 0x00, 0x05, 0x01, 0x01, 0x01, 0x00, 0x05, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void get_random_empty_tile_location(point &pos) {
	while (1) {
		pos.x = blit::random() % MAP_WIDTH;
		pos.y = blit::random() % MAP_HEIGHT;
		if (map.has_flag(pos, TileFlags::NO_GRASS)) continue;
		return;
	}
}

/* setup */
void init() {
	//set_screen_mode(screen_mode::hires);
	//printf("Init: STARTED\n");
	//engine::render = ::render;
	//engine::update = ::update;
	fb.sprites = spritesheet::load(packed_data);

	map.add_layer("walls", map_data_walls);
	map_layer_walls = &map.layers["walls"];
	map_layer_walls->add_flags({ 1, 2, 3, 4, 5 }, TileFlags::WALL);
	map_layer_walls->add_flags({ 1, 2, 3, 4, 5 }, TileFlags::NO_GRASS);

	map.add_layer("floor", map_data_floor);
	map_layer_floor = &map.layers["floor"];
	map_layer_floor->add_flags({ 3, 4, 5 }, TileFlags::NO_GRASS);

	//my_sprites.s.load_from_packed(packed_data);

	player1.direction.y = -1;
	player1.position.x = 3.5;
	player1.position.y = 3.5;

	tan_half_fov = tan(HALF_FOV);

	for (int x = 0; x < SCREEN_WIDTH; x++) {
		lut_camera_displacement[x] = (float)(2 * x) / SCREEN_WIDTH - 1;
	}

	srand(0x32bl);
	//stars.resize((const int)num_stars);
	for (int s = 0; s < num_stars; s++) {
		stars[s].position.x = blit::random() % 360;
		stars[s].position.y = blit::random() % (HORIZON / 2);
		stars[s].brightness = 32 + (blit::random() % 128);
	}

	point tile;
	vec2 offset;
	//map_sprites.resize((const int)num_sprites);
	for (int s = 0; s < num_sprites; s++) {
		int texture = blit::random() % 100;
		if (texture == 0) {
			map_sprites[s].texture = 0;
		}
		else if (texture == 1) {
			map_sprites[s].texture = 1;
		}
		else if (texture == 2) {
			map_sprites[s].texture = 2;
		}
		else if (texture == 3) {
			map_sprites[s].texture = 3;
		}
		else {
			map_sprites[s].texture = 4 + (texture % 3);
		}
		get_random_empty_tile_location(tile);
		offset.x = (float)blit::random() / 4294967295.0f;
		offset.y = (float)blit::random() / 4294967295.0f;
		map_sprites[s].position.x = tile.x + offset.x;
		map_sprites[s].position.y = tile.y + offset.y;
		map_sprites[s].color = blit::random() % 3;
	}

	//printf("Init: FINISHED\n");
}

// TODO: should be in the vec2 class
vec2 rotate_point(vec2 p, vec2 v) {
	float a = atan2(v.y, v.x);
	return rotate_vector(p, a);
}

// TODO: should be in the vec2 class
vec2 rotate_vector(vec2 v, float a) {
	float c = cos(a);
	float s = sin(a);
	float tx = v.x * c - v.y * s;
	float ty = v.x * s + v.y * c;
	return vec2{ tx, ty };
}

// TODO: should be in the vec2 class
float measure_vector(vec2 v) {
	return sqrt((v.x * v.x) + (v.y * v.y));
}

uint32_t elapsed;

void update(uint32_t time) {
	//printf("Update: STARTED\n");
	static uint32_t last_time;
	elapsed = time - last_time;
	last_time = time;
	static vec2 size(0.2f, 0.2f);
	static vec2 rmove(0, 0);
	vec2 move(0, 0);

	int check_tile = 0;

	if (pressed(button::DPAD_UP)) {
		move.x = 0.02f;
	}
	else if (pressed(button::DPAD_DOWN)) {
		move.x = -0.02f;
	}
	else if (joystick.y < -0.1f || joystick.y > 0.1f) {
		move.x = -joystick.y * 0.02f;
	}

	/*if (blit::joystick.x != 0) {
		float x = blit::joystick.x / (float)30.0;
		flip_doom_guy = blit::joystick.x > 0;
		player1.direction = rotate_vector(player1.direction, x);
	}

	if (blit::joystick.y != 0) {
		float y = blit::joystick.y / (float)30.0;
		move.x = -y;
	}*/

	rmove.x = move.x * player1.direction.x - move.y * player1.direction.y;
	rmove.y = move.x * player1.direction.y + move.y * player1.direction.x;

	if (rmove.x < 0) {
		int bound = floor(player1.position.x + rmove.x - size.x);

		check_tile = check_tile | map.get_flags(point(bound, floor(player1.position.y)));
		if (rmove.y > 0) {
			check_tile = check_tile | map.get_flags(point(bound, floor(player1.position.y - size.y)));
		}
		else if (rmove.y < 0) {
			check_tile = check_tile | map.get_flags(point(bound, floor(player1.position.y + size.y)));
		}
	}
	else if (rmove.x > 0) {
		int bound = floor(player1.position.x + rmove.x + size.x);

		check_tile = check_tile | map.get_flags(point(bound, floor(player1.position.y)));
		if (rmove.y > 0) {
			check_tile = check_tile | map.get_flags(point(bound, floor(player1.position.y - size.y)));
		}
		else if (rmove.y < 0) {
			check_tile = check_tile | map.get_flags(point(bound, floor(player1.position.y + size.y)));
		}
	}

	if ((check_tile & TileFlags::WALL) == 0) {
		player1.position.x += rmove.x;
	}

	check_tile = 0;

	if (rmove.y < 0) {
		int bound = floor(player1.position.y + rmove.y - size.y);

		check_tile = check_tile | map.get_flags(point(floor(player1.position.x), bound));
		if (rmove.x > 0) {
			check_tile = check_tile | map.get_flags(point(floor(player1.position.x - size.x), bound));
		}
		else if (rmove.x < 0) {
			check_tile = check_tile | map.get_flags(point(floor(player1.position.x + size.x), bound));
		}
	}
	else if (rmove.y > 0) {
		int bound = floor(player1.position.y + rmove.y + size.y);

		check_tile = check_tile | map.get_flags(point(floor(player1.position.x), bound));
		if (rmove.x > 0) {
			check_tile = check_tile | map.get_flags(point(floor(player1.position.x - size.x), bound));
		}
		else if (rmove.x < 0) {
			check_tile = check_tile | map.get_flags(point(floor(player1.position.x + size.x), bound));
		}
	}

	if ((check_tile & TileFlags::WALL) == 0) {
		player1.position.y += rmove.y;
	}

	if (pressed(button::A)) {
		bug_spraying = 1;
	}
	else
	{
		bug_spraying = 0;
	}

	if (pressed(button::DPAD_LEFT)) {
		flip_doom_guy = false;
		player1.direction = rotate_vector(player1.direction, -0.02);
	}
	else if (pressed(button::DPAD_RIGHT)) {
		flip_doom_guy = true;
		player1.direction = rotate_vector(player1.direction, 0.02);
	}
	else if (joystick.x < -0.1f || joystick.x > 0.1f) {
		player1.direction = rotate_vector(player1.direction, joystick.x * 0.02f);
		flip_doom_guy = joystick.x > 0;
	}

	player1.direction.normalize();

	// update the orientation of the player camera plane
	player1.camera = vec2(-player1.direction.y, player1.direction.x);
	//printf("Update: FINISHED\n");
}

void render(uint32_t time) {
	//printf("Render: STARTED\n");
	uint32_t ms_start = now();

	// clear the mask
	m.alpha = 255;
	m.pen(rgba(0));
	m.clear();

	// clear the canvas
	fb.alpha = 255;
	fb.mask = nullptr;
	fb.pen(rgba(22, 21, 31));
	fb.clear();

	//printf("Render: SKY\n");
	render_sky();

	//printf("Render: STARS\n");
	render_stars();

	// TODO ???		
	//printf("Render: WORLD\n");
	render_world(time);
	/*
	fb.mask = &m;
	blur(5);
	fb.pen(rgba(0, 0, 0, 140));
	fb.clear();

	fb.mask = nullptr;*/

	//edges();

		blur(1);

	fb.pen(rgba(10, 36, 24));
	fb.mask = nullptr;
	for (int y = 0; y < m.bounds.h; y++) {
		for (int x = 0; x < m.bounds.w; x++) {
			uint8_t v = *m.ptr(x, y);
			fb.alpha = v;
			fb.pixel(point(x, y));
		}
	}
	fb.alpha = 255;

	// TODO ???	


	//printf("Render: SPRITES\n");
	render_sprites(time);

	// draw bug spray    
	//rect ss_spray_rect(40, 160 - 32, 24, 32);
	int offset = int(sin((player1.position.x + player1.position.y) * 4) * 3); // bob

	fb.sprite(rect(5, 16, 3, 4), point(SCREEN_WIDTH - 48, VIEW_HEIGHT - 30 + offset));


	//fb.blit(&ss, ss_spray_rect, point(SCREEN_WIDTH - 48, VIEW_HEIGHT - 30 + offset));

	// draw the HUD
	fb.pen(rgba(37, 36, 46));
	fb.rectangle(rect(0, 120 - 24, 160, 24));
	//rect ss_hud_rect(0, 160 - 24, 8, 8);
	for (int x = 0; x < 160 / 8; x++) {
		//fb.blit(&ss, ss_hud_rect, point(x * 8, 120 - 24));
		fb.sprite(340, point(x * 8, 120 - 24));
	}

	//rect ss_heart_filled_rect(8, 128, 8, 8);
	//rect ss_heart_empty_rect(16, 128, 8, 8);
	for (int x = 0; x < 4; x++) {
		//fb.blit(&ss, x > 1 ? ss_heart_empty_rect : ss_heart_filled_rect, point(32 + x * 10, 120 - 16));
		fb.sprite(x > 1 ? 322 : 321, point(32 + x * 10, 120 - 16));
	}

	// draw DOOM guy (phil)
	rect ss_guy_rect(160 - 72, 160 - 32, 24, 32);
	//fb.blit(&ss, ss_guy_rect, point(0, 120 - 32), flip_doom_guy);
	fb.sprite(rect(11, 16, 3, 4), point(0, 120 - 32), flip_doom_guy ? sprite_transform::HORIZONTAL : 0);

	//fb.mask = &m;
	//fb.pen(rgba(255, 0, 0, 255));
	//fb.rectangle(rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	//fb.blit(&m, rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), point(0, 0));

	uint32_t ms_end = now();

	// draw FPS meter
	fb.mask = nullptr;
	fb.pen(rgba(255, 0, 0));
	for (unsigned int i = 0; i < (ms_end - ms_start); i++) {
		fb.pen(rgba(i * 5, 255 - (i * 5), 0));
		fb.rectangle(rect(i * 3 + 1, 117, 2, 2));
	}
	//printf("Render: FINISHED\n");
}

void render_sky() {
	for (uint16_t column = 0; column < SCREEN_WIDTH; column++) {
		// Moved to a lookup table
		//float camera_displacement = (float)(2 * column) / SCREEN_WIDTH - 1;

		// the current ray direction is the player direction, plus the camera direction multiplied by the displacement
		vec2 ray(
			player1.direction.x + player1.camera.x * lut_camera_displacement[column],
			player1.direction.y + player1.camera.y * lut_camera_displacement[column]
		);

		ray.normalize();

		// TODO: for the API? 
		// Convert the facing vector to an angle in degrees
		//float r = abs(atan2(ray.x, ray.y) * 180.0 / M_PI);

		float r = atan2f(ray.x, ray.y);
		r = (r > 0.0f ? r : (2.0f * float(M_PI) + r)) * 360.0f / (2.0f * float(M_PI));


		point uv(24 + (int(r * 3.0f) % 16), 160 - 32);

		fb.stretch_blit_vspan(fb.sprites, uv, 32, point(column, 0), HORIZON); // TODO: blit from spritesheet?

		// Apply radial darkness to simulate directional sunset
		uint8_t fade = std::max(-120, std::min(120, abs(int(r) - 120))) + 120;  // calculate a `fog` based on angle
		fb.pen(rgba(12, 33, 52, fade));
		fb.line(point(column, 0), point(column, HORIZON));
	}
}

void render_stars() {
	// Get the player's facing angle in degrees from 0 to 359
	float r = atan2f(player1.direction.x, player1.direction.y);
	r = (r > 0.0f ? r : (2.0f * float(M_PI) + r)) * 360.0f / (2.0f * float(M_PI));

	for (int s = 0; s < num_stars; s++) {
		star *sp = &stars[s];

		// If the stars radial X position is within our field of view
		if ((180 - abs(abs(r - sp->position.x) - 180)) < 45) {
			// Get the difference between the star and player angle as degrees, signed
			int x = r - sp->position.x + 180;
			x = x - floor(float(x) / 360.0f) * 360;
			x -= 180;

			// Convert the degrees to screen columns
			x = 80 + (x / 45.0f) * 80;
			fb.pen(rgba(255, 255, 255, sp->brightness));
			fb.pixel(point(
				x,
				sp->position.y * 2
			));
		}
	}

	/*std::ostringstream message;
	message << r << ":" << x << "c:" << count;
	fb.pen(rgba(255, 255, 255));
	fb.text(message.str().c_str(), rect(0, HORIZON, 160, 20));*/

}

void render_world(uint32_t time) {
	//TileFacing tfacing = TileFacing::NONE;
	//TileFacing last_tfacing = TileFacing::NONE;
	float perpendicular_wall_distance, wall_x;
	point map_location_g((int32_t)floor(player1.position.x), (int32_t)floor(player1.position.y));
	point map_location;
	point last_map_location(-1, -1);
		int last_side = -1;

	float last_wall_distance = 0;

	//printf("render_world: populate visibility map\n");
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			visibility_map[x + y * MAP_WIDTH] = false;
		}
	}
	visibility_map[map_location_g.x + map_location_g.y * MAP_WIDTH] = true;

	//printf("render_world: start column scanning\n");
	for (uint16_t column = 0; column < SCREEN_WIDTH; column++) { // trace SCREEN_WIDTH rays from left to right
		// calculate the amount we need to scale the plane_x/y camera displacement
		// this gives us a step along the camera displacement that corresponds to the current ray
		map_location = map_location_g;

		// Moved to lookup table
		//camera_displacement = (float)(2 * column) / SCREEN_WIDTH - 1;

		// the current ray direction is the player direction, plus the camera direction multiplied by the displacement
		vec2 ray(
			player1.direction.x + player1.camera.x * lut_camera_displacement[column],
			player1.direction.y + player1.camera.y * lut_camera_displacement[column]
		);

		vec2 delta_dist(
			(float)abs(1.0f / ray.x),
			(float)abs(1.0f / ray.y)
		);

		vec2 side_dist(0, 0);
		int8_t step_x, step_y;

		if (ray.x < 0) {
			step_x = -1;
			side_dist.x = (player1.position.x - map_location.x) * delta_dist.x;
		}
		else
		{
			step_x = 1;
			side_dist.x = (map_location.x + 1.0f - player1.position.x) * delta_dist.x;
		}

		if (ray.y < 0) {
			step_y = -1;
			side_dist.y = (player1.position.y - map_location.y) * delta_dist.y;
		}
		else
		{
			step_y = 1;
			side_dist.y = (map_location.y + 1.0f - player1.position.y) * delta_dist.y;
		}

		bool hit = false;

		//printf("render_world: cast ray for column %d\n", column);
		int side = 0;
		for (int s = 0; s < MAX_RAY_STEPS; s++) {

			if (side_dist.x < side_dist.y) {
				side_dist.x += delta_dist.x;
				map_location.x += step_x;
				side = 0;
			}
			else
			{
				side_dist.y += delta_dist.y;
				map_location.y += step_y;
				side = 1;
			}

			//printf("render_world: update visibility map %d:%d\n", map_location.x, map_location.y);
			if (map_location.x + map_location.y * MAP_WIDTH < MAP_WIDTH * MAP_HEIGHT) {
				visibility_map[map_location.x + map_location.y * MAP_WIDTH] = true;
			}
			//printf("render_world: update visibility map %d:%d: DONE!\n", map_location.x, map_location.y);

				//tile = get_map_tile(map_location);

				//if (column == 80) {
				//	tile_in_view = map_location;
				//}
				/*if (tile & MAP_TILE_SOLID) {
					break;
				}*/
			if (map.has_flag(map_location, TileFlags::WALL)) {
				hit = true;
				break;
			}
		}

		if (hit) {
			//printf("render_world: resolving hit %d:%d\n", map_location.x, map_location.y);
			uint8_t texture_wall = map_layer_walls->tile_at(map_location) - 1;// tile & 0x0f;

			if (side == 0) {
				perpendicular_wall_distance = ((float)map_location.x - player1.position.x + (1 - step_x) / 2.0f) / ray.x;
				wall_x = player1.position.y + perpendicular_wall_distance * ray.y;
				/*if (ray.x > 0) {
					tfacing = TileFacing::WEST;
				}
				else
				{
					tfacing = TileFacing::EAST;
				}*/
			}
			else {
				perpendicular_wall_distance = ((float)map_location.y - player1.position.y + (1 - step_y) / 2.0f) / ray.y;
				wall_x = player1.position.x + perpendicular_wall_distance * ray.x;
				/*if (ray.y > 0) {
					tfacing = TileFacing::NORTH;
				}
				else
				{
					tfacing = TileFacing::SOUTH;
				}*/
			}

			wall_x -= floor(wall_x);

			// While the perpendicular wall distance prevents fish-eye effect, generally we want
			// lighting and distance based overlay effects to use the "real" wall distance
			// otherwise the player can see further out of the corner of their eye
			// real_wall_distance = perpendicular_wall_distance / cos(atan2(player1.direction.x, player1.direction.y)) - atan2(ray.x, ray.y);

			int wall_half_height = (int)((float)HORIZON / perpendicular_wall_distance);
			int start_y = HORIZON - wall_half_height;
			int end_y = HORIZON + wall_half_height;

			//printf("render_world: updating z_buffer\n");
			z_buffer[column] = perpendicular_wall_distance;

			//m.pen(int(alpha));
			m.pen(200);

			float line_distance = abs(perpendicular_wall_distance - last_wall_distance);

			int width = wall_half_height / 8.0f;

			if (column > 0 && (side != last_side) && line_distance < 0.5f && !(map_location.x == last_map_location.x && map_location.y == last_map_location.y)) {

				for (int c = column - width; c < column + width; c++) {
					int alpha = (abs(column - c) * 160) / width;
					m.pen(160 - alpha);
					m.line(point(c, start_y), point(c, end_y));
				};
				/*m.rectangle(rect(
					point(column - width, start_y),
					point(column + width, end_y)
				));*/
				//m.line(point(column-1, start_y), point(column-1, end_y));
				//m.line(point(column, start_y), point(column, end_y));
				//m.line(point(column+1, start_y), point(column+1, end_y));
			}
			else {
				for (int r = end_y - width; r < end_y + width; r++) {
					int alpha = (abs(end_y - r) * 160) / width;
					m.pen(160 - alpha);
					m.pixel(point(column, r));
					/*m.rectangle(rect(
						point(column, end_y - width - width + 2),
						point(column + 1, end_y + 2)
					));*/
				}
			}

			//last_tfacing = tfacing;
			last_side = side;
			last_wall_distance = perpendicular_wall_distance;
			last_map_location = map_location;

			/* draw the walls */

		//printf("render_world: drawing vertical wall slice\n");
			// TODO: add mipmap support? automatic based on scale?          

			// texture_wall
			//
			//  0 = mossy stone
			//  1 = crumbled brick
			//  2 = good brick
			//  3 = spoopy door
			//  4 = good brick support
			uint16_t texture_offset_x = texture_wall * 32;
			point uv = point(uint8_t(wall_x * 32.0f) + texture_offset_x, 0);

			//if ((time >> 2) % 160 == column) {


			fb.stretch_blit_vspan(fb.sprites, uv, 32, point(column, start_y), end_y - start_y); // TODO Blit from Spritesheet


			//}
			/*int8_t fade = (int)(wall_distance * 255.0f);  // calculate a `fog` based on distance
			if (side == 1) {
				fade *= 0.9f;
			}*/

			/*if (side == 1) {
				alpha *= 0.9f;
			}*/

			//printf("render_world: distance shading\n");
			float wall_distance = perpendicular_wall_distance / MAX_RAY_STEPS;
			float alpha = wall_distance * 255.0f;
			fb.pen(rgba(0, 0, 0, int(alpha)));
			fb.line(point(column, start_y), point(column, end_y));


			/*m.pen(rgba(255));
			m.line(point(column, start_y), point(column, end_y));
			*/





			vec2 floor_wall(map_location.x, map_location.y);

			if (side == 0 && ray.x > 0) {
				floor_wall.y += wall_x;
			}
			else if (side == 0 && ray.x < 0) {
				floor_wall.x += 1.0f;
				floor_wall.y += wall_x;
			}
			else if (side == 1 && ray.y > 0) {
				floor_wall.x += wall_x;
			}
			else {
				floor_wall.x += wall_x;
				floor_wall.y += 1.0f;
			}

			//printf("render_world: drawing floor\n");
				// Draw the floor
			for (int y = end_y + 1; y < VIEW_HEIGHT + 1; y++) {
				float distance = (float)VIEW_HEIGHT / (2.0f * y - VIEW_HEIGHT);
				float weight = distance / perpendicular_wall_distance;

				vec2 current_floor(
					weight * floor_wall.x + (1.0f - weight) * player1.position.x,
					weight * floor_wall.y + (1.0f - weight) * player1.position.y
				);

				// Get the tile-relative x/y texture coordinates
				point tile_uv(
					(current_floor.x - floor(current_floor.x)) * 32,
					(current_floor.y - floor(current_floor.y)) * 32
				);

				uint8_t floor_texture = map_layer_floor->tile_at(point(int(current_floor.x), int(current_floor.y))) - 1;
				//uint8_t floor_texture = get_map_tile(point(int(current_floor.x), int(current_floor.y))) & 0x0f;

				point floor_texture_sprite(
					32 * floor_texture,
					32
				);
				// Get the distance from the player to the point on the floor
				// and use this to create a distance shadowing effect
				// dist = current_floor - player1.position;
				// p_distance = dist.length();

				int fragment_x = floor_texture_sprite.x + tile_uv.x;
				int fragment_y = floor_texture_sprite.y + tile_uv.y;


				uint8_t fragment_c_idx = *fb.sprites->ptr(fragment_x, fragment_y);

				rgba fragment_c = fb.sprites->palette[fragment_c_idx];
				//if (time >> 2 % 360 == column) {
				fb.pen(fragment_c);
				fb.pixel(point(column, y - 1));
				//}

				float floor_distance = distance / MAX_RAY_STEPS;

				fb.pen(rgba(0, 0, 0, int(floor_distance * 255.0f)));
				fb.pixel(point(column, y - 1));
			}
		}
	}
}

void render_sprites(uint32_t time) {

	float inverse_det = 1.0f / (player1.camera.x * player1.direction.y - player1.direction.x * player1.camera.y);

	// Calculate distance from player to each sprite
	for (int i = 0; i < num_sprites; i++) {
		vec2 sprite_distance(
			map_sprites[i].position.x - player1.position.x,
			map_sprites[i].position.y - player1.position.y
		);
		map_sprites[i].distance = (sprite_distance.x * sprite_distance.x) + (sprite_distance.y * sprite_distance.y);
	}

	// sort the sprites by distance
	std::sort(map_sprites.begin(), map_sprites.end());

	for (int i = 0; i < num_sprites; i++) {
		sprite *psprite = &map_sprites[i];

		rgba cols_a[]{
			rgba(0x15, 0x98, 0x5d, 200),
			rgba(0x35, 0xA8, 0x3d, 200),
			rgba(0x45, 0x88, 0x2d, 200)
		};

		rgba cols_b[]{
			rgba(0x00, 0x7f, 0x43, 200),
			rgba(0x20, 0x6f, 0x33, 200),
			rgba(0x30, 0x8f, 0x23, 200)
		};

		fb.sprites->palette[11] = cols_a[psprite->color];
		fb.sprites->palette[12] = cols_b[psprite->color];


		if (visibility_map[int(psprite->position.x) + int(psprite->position.y) * MAP_WIDTH] == 0) continue;

		// Give the larger sprites a better view distance
		float max_distance = (psprite->texture == 0 || psprite->texture == 1) ? 64.0 : 16.0;

		float distance = std::min(max_distance, psprite->distance) / max_distance;
		if (distance == 1.0f) continue;

		// Get the player-relative position of the sprite
		vec2 relative_position = psprite->position - player1.position;

		vec2 screen_transform(
			inverse_det * (player1.direction.y * relative_position.x - player1.direction.x * relative_position.y),
			inverse_det * (-player1.camera.y * relative_position.x + player1.camera.x * relative_position.y)
		);

		if (screen_transform.y < 0)
			continue;


		// TODO:: palette change
		//int color_offset = spr.color * 4;

		rect sprite_bounds[7] = {
			rect(0, 64, 28, 64),   // Full-grown tree
			rect(28, 74, 28, 54),  // Mature tree
			rect(56, 94, 28, 34),  // Tall shrub
			rect(56, 70, 28, 24),  // Short shrub
			rect(48, 65, 8, 8),    // Tall grass
			rect(38, 66, 9, 7),    // Mid grass
			rect(30, 68, 7, 5)     // Smol grass
		};

		rect bounds = sprite_bounds[psprite->texture];

		int sprite_height = abs(int(bounds.h * SPRITE_SCALE / screen_transform.y));
		int sprite_width = ((float)bounds.w / (float)bounds.h) * sprite_height;

		int sprite_top_y = ((VIEW_HEIGHT - bounds.h) * SPRITE_SCALE) / screen_transform.y;

		// Get the screen-space position of the sprites base on the floor
		vec2 screen_pos(
			int((SCREEN_WIDTH / 2) * (1 + screen_transform.x / screen_transform.y)),
			HORIZON + (HORIZON / screen_transform.y)
		);

		/* DEBUG: Plot the sprite's base with a red dot
		fb.alpha = 255 - int(255 * distance);
		fb.pen(rgba(255, 0, 0));
		fb.pixel(point(screen_pos.x, screen_pos.y));
		*/

		// offset screen coordinate with sprite bounds
		screen_pos -= vec2(sprite_width / 2, sprite_height);

		//fb.stretch_blit(&my_sprites, bounds, rect(screen_pos.x, screen_pos.y, sprite_width, sprite_height));

		for (int x = std::max(0, int(screen_pos.x)); x < std::min(SCREEN_WIDTH, int(screen_pos.x + sprite_width)); x++) {
			if (screen_transform.y > z_buffer[x]) continue;

			//if ((time >> 2) % 160 != x) { continue; }

			vec2 uv(
				bounds.x + ((float(x - screen_pos.x) / float(sprite_width)) * bounds.w),
				bounds.y
			);

			fb.stretch_blit_vspan(fb.sprites, uv, bounds.h, point(x, screen_pos.y), sprite_height); // TODO: blit from spritesheet?
		}
		fb.sprites->palette[11] = rgba(0x15, 0x98, 0x5d, 200);
		fb.sprites->palette[12] = rgba(0x00, 0x7f, 0x43, 200);
	}
}

void update_player_camera_plane(void) {
	//vec2 plane(-player1.direction.y, player1.direction.x);

	//plane = rotate_vector(plane, M_PI_H);
	//float magnitude = plane.length() / tan_half_fov;
	//player1.camera.x = plane.x / magnitude;
	//player1.camera.y = plane.y / magnitude;

	//plane.rotate(float(M_PI_H));
	//plane.rotate90cw();
	//plane.normalize();

	// If the player's camera plane is 90 degrees to their facing direction,
	// then since math.cos(PI / 2) =~ 0 and math.sin(PI / 2) =~ 1.0
	// a 90 degree rotation is just (-y, x)
}

void edges() {
	uint8_t *p = (uint8_t *)m.data + 160;
	for (uint16_t y = 1; y < 119; y++) {
		p++;

		for (uint16_t x = 1; x < 159; x++) {
			uint8_t v1 = abs(*(p + 1) - *p);
			uint8_t v2 = abs(*(p + 160) - *p);
			uint8_t d = v1 > v2 ? v1 : v2;
			*p++ = d < 3 ? 0 : 255;
		}

		p++;
	}

	p = (uint8_t *)m.data + (120 * 160) - 1 - 160;
	for (uint16_t y = 1; y < 119; y++) {
		p--;

		for (uint16_t x = 1; x < 159; x++) {
			uint8_t v1 = abs(*(p - 1) - *p);
			uint8_t v2 = abs(*(p - 160) - *p);
			uint8_t d = v1 > v2 ? v1 : v2;
			*p-- = d < 3 ? 0 : 255;
		}

		p--;
	}
}

void blur(uint8_t passes) {
	uint8_t last;

	for (uint8_t pass = 0; pass < passes; pass++) {
		uint8_t *p = (uint8_t *)m.data;
		for (uint16_t y = 0; y < 120; y++) {
			last = *p;
			p++;

			for (uint16_t x = 1; x < 159; x++) {
				*p = (*(p + 1) + last + *p + *p) >> 2;
				last = *p;
				p++;
			}

			p++;
		}
	}

	// vertical      
	for (uint8_t pass = 0; pass < passes; pass++) {
		for (uint16_t x = 0; x < 160; x++) {
			uint8_t *p = (uint8_t *)m.data + x;

			last = *p;
			p += 160;

			for (uint16_t y = 1; y < 119; y++) {
				*p = (*(p + 160) + last + *p + *p) >> 2;
				last = *p;
				p += 160;
			}
		}
	}
}

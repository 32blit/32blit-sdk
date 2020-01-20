
#include <string>
#include <memory>
#include <cstdlib>
#include <array>

#include "tunnel.hpp"

using namespace blit;

const uint16_t screen_width = 160;
const uint16_t screen_height = 120;

spritesheet *ss_ship = spritesheet::load(packed_data_ship);

/* setup */
void init() {
	fb.alpha = 255;
	fb.mask = nullptr;
	fb.pen(rgba(0, 0, 0, 0));
	fb.clear();

	fb.sprites = spritesheet::load(packed_data);
}

void tunnel_test(uint32_t time_ms) {
	point mipmap_offset[5] = {
		point(0, 0),
		point(128, 0),
		point(128, 64),
		point(128, 96),
		point(128, 112)
	};
	uint8_t mipmap_size[5] = {
		128,
		64,
		32,
		16,
		8
	};

	float x = time_ms / 1000.0;
	
	float y = 1.0;

	for (int scanline = 0; scanline < fb.bounds.w; scanline++) {

		float z = 3.0; // Distance from our imaginary wall plane
		z += sin((time_ms + scanline) / 1000.0f);
		y = sin((time_ms + scanline) / 200.0f);

		int offset = scanline;

		int wall_height = fb.bounds.h / z;
		int wall_offset_top = (fb.bounds.h - wall_height) / 2;
		float wall_offset = y * (wall_height / 4.0f);

		wall_offset_top -= wall_offset;

		int mipmap_index = 0;
		if (wall_height <= 8) {
			mipmap_index = 4;
		}
		if (wall_height <= 16) {
			mipmap_index = 3;
		}
		if (wall_height <= 32) {
			mipmap_index = 2;
		}
		if (wall_height <= 64) {
			mipmap_index = 1;
		}
			
		point texture_origin = mipmap_offset[mipmap_index];
		uint8_t texture_size = mipmap_size[mipmap_index];

		for (int wall_y = 0; wall_y < wall_height; wall_y++) {
			vec2 uv(
				(float(scanline) / wall_height) + x,
				(float(wall_y) / wall_height)
			);

			uv.x -= floor(uv.x);
			uv.y -= floor(uv.y);


			uint8_t *fragment_c = fb.sprites->ptr(point(
				texture_origin.x + (uv.x * texture_size),
				texture_origin.y + (uv.y * texture_size)
			));

			fb.pen(fb.sprites->palette[*fragment_c]);
			fb.alpha = 255;
			fb.pixel(point(
				scanline,
				wall_offset_top + wall_y
			));
		}

		for (int ceil_y = 0; ceil_y < wall_offset_top; ceil_y++) {

			float distance = (float)fb.bounds.h / (2.0f * (fb.bounds.h - ceil_y - wall_offset) - fb.bounds.h);
			float weight = distance / z;

			float t_size = float(fb.bounds.h) / distance;

			int mipmap_index = 0;
			if (t_size <= 8) {
				mipmap_index = 4;
			}
			if (t_size <= 16) {
				mipmap_index = 3;
			}
			if (t_size <= 32) {
				mipmap_index = 2;
			}
			if (t_size <= 64) {
				mipmap_index = 1;
			}

			point texture_origin = mipmap_offset[mipmap_index];
			uint8_t texture_size = mipmap_size[mipmap_index];


			vec2 uv(
				(float(scanline) / wall_height) + x,
				0.5
			);

			uv.x = uv.x * weight;
			uv.x += x * (1.0f - weight);

			uv.y = uv.y * weight;
			uv.y += z * (1.0f - weight);

			uv.x -= floor(uv.x);
			uv.y -= floor(uv.y);

			uint8_t *fragment_c = fb.sprites->ptr(point(
				texture_origin.x + (uv.x * texture_size),
				texture_origin.y + (uv.y * texture_size)
			));

			fb.pen(fb.sprites->palette[*fragment_c]);
			fb.alpha = 200;
			fb.pixel(point(scanline, ceil_y));
		}

		for (int floor_y = wall_offset_top + wall_height; floor_y < fb.bounds.h; floor_y++) {

			float distance = (float)fb.bounds.h / (2.0f * (floor_y + wall_offset) - fb.bounds.h);
			float weight = distance / z;

			float t_size = float(fb.bounds.h) / distance;

			int mipmap_index = 0;
			if (t_size <= 8) {
				mipmap_index = 4;
			}
			if (t_size <= 16) {
				mipmap_index = 3;
			}
			if (t_size <= 32) {
				mipmap_index = 2;
			}
			if (t_size <= 64) {
				mipmap_index = 1;
			}

			point texture_origin = mipmap_offset[mipmap_index];
			uint8_t texture_size = mipmap_size[mipmap_index];


			vec2 uv(
				(float(scanline) / wall_height) + x,
				0.1
			);

			uv.x = uv.x * weight;
			uv.x += x * (1.0f - weight);

			uv.y = uv.y * weight;
			uv.y += z * (1.0f - weight);

			uv.x -= floor(uv.x);
			uv.y -= floor(uv.y);

			uint8_t *fragment_c = fb.sprites->ptr(point(
				texture_origin.x + (uv.x * texture_size),
				texture_origin.y + (uv.y * texture_size)
			));

			fb.pen(fb.sprites->palette[*fragment_c]);
			fb.alpha = 200;
			fb.pixel(point(scanline, floor_y));

		}
	}

}

void tunnel_test1(uint8_t time_ms) {

	int max_stripe_height = (fb.bounds.h / 3);
	int min_stripe_height = 40;

	for (int scanline = 0; scanline < fb.bounds.w; scanline++) {
		fb.alpha = 255;
		int offset = scanline * 2;
		//float time_offset = (time_ms + offset) / 100.0f;
		float x = (sin(float(time_ms + offset) / 1000.0f) + 1.0f) / 2.0f;
		float y = (sin(float(time_ms + offset + 1000) / 2000.0f) + 1.0f) / 2.0f;

		float stripe_height = min_stripe_height + max_stripe_height * y;
		float stripe_offset_top = (fb.bounds.h / 4) * x;

		float t_scale = float(min_stripe_height + max_stripe_height) / stripe_height;

		/*fb.pen(rgba(255, 200, 200));
		fb.line(
			point(scanline, stripe_offset_top),
			point(scanline, stripe_offset_top + stripe_height)
		);*/

		vec2 c(((scanline * t_scale) + (time_ms / 5.0f)), 0);

		fb.stretch_blit_vspan(fb.sprites, point(int(c.x) % 128, int(c.y) % 128), 128, point(scanline, stripe_offset_top), stripe_height);


		for (int f = 0; f < fb.bounds.h - stripe_height - stripe_offset_top - 1; f++) {
			float distance = (float)(fb.bounds.h - stripe_height - stripe_offset_top - 1) / f;

			c.y = distance * stripe_height;
			/*while (c.y < 0) {
				c.y = 127 - c.y;
			}
			while (c.y > 127) {
				c.y -= 127;
			}*/

			c.x -= (scanline - (fb.bounds.w / 2)) / 200.0f;
			/*while (c.x < 0) {
				c.x = 127 - c.x;
			}
			while (c.x > 127) {
				c.x -= 127;
			}*/


			uint8_t *fragment_c = fb.sprites->ptr(int(c.x) % 128, int(c.y) % 128);

			fb.pen(fb.sprites->palette[*fragment_c]);
			fb.alpha = 200;
			fb.pixel(point(scanline, (stripe_offset_top + stripe_height) + f - 1));
		}
	}

}

void render(uint32_t time_ms) {
	uint32_t ms_start = now();

	fb.pen(rgba(0, 0, 0, 255));
	fb.clear();

	tunnel_test(time_ms);

	fb.alpha = 255;

	int x = 20 + (sin(time_ms / 1000.0f) * 10.0f);
	int y = 40 + (sin(time_ms / 1500.0f) * 30.0f);

	fb.blit(ss_ship, rect(0, 0, 64, 32), point(x, y));

	uint32_t ms_end = now();
	fb.mask = nullptr;
	fb.pen(rgba(255, 0, 0));
	for (int i = 0; i < (ms_end - ms_start); i++) {
		fb.pen(rgba(i * 5, 255 - (i * 5), 0));
		fb.rectangle(rect(i * 3 + 1, fb.bounds.h - 3, 2, 2));
	}
}

void update(uint32_t time) {

}

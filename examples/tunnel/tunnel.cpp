
#include <string>
#include <memory>
#include <cstdlib>
#include <array>

#include "tunnel.hpp"
#include "assets.hpp"

using namespace blit;

Surface *ss_ship = Surface::load(packed_data_ship);

/* setup */
void init() {
	screen.alpha = 255;
	screen.mask = nullptr;
	screen.pen = Pen(0, 0, 0, 0);
	screen.clear();

	screen.sprites = Surface::load(packed_data);
}

void tunnel_test(uint32_t time_ms) {
	Point mipmap_offset[5] = {
		Point(0, 0),
		Point(128, 0),
		Point(128, 64),
		Point(128, 96),
		Point(128, 112)
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

	for (int scanline = 0; scanline < screen.bounds.w; scanline++) {

		float z = 3.0; // Distance from our imaginary wall plane
		z += sinf((time_ms + scanline) / 1000.0f);
		y = sinf((time_ms + scanline) / 200.0f);

		int wall_height = screen.bounds.h / z;
		int wall_offset_top = (screen.bounds.h - wall_height) / 2;
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

		Point texture_origin = mipmap_offset[mipmap_index];
		uint8_t texture_size = mipmap_size[mipmap_index];

		for (int wall_y = 0; wall_y < wall_height; wall_y++) {
			Vec2 uv(
				(float(scanline) / wall_height) + x,
				(float(wall_y) / wall_height)
			);

			uv.x -= floorf(uv.x);
			uv.y -= floorf(uv.y);


			uint8_t *fragment_c = screen.sprites->ptr(Point(
				texture_origin.x + (uv.x * texture_size),
				texture_origin.y + (uv.y * texture_size)
			));

			screen.pen = screen.sprites->palette[*fragment_c];
			screen.alpha = 255;
			screen.pixel(Point(
				scanline,
				wall_offset_top + wall_y
			));
		}

		for (int ceil_y = 0; ceil_y < wall_offset_top; ceil_y++) {

			float distance = (float)screen.bounds.h / (2.0f * (screen.bounds.h - ceil_y - wall_offset) - screen.bounds.h);
			float weight = distance / z;

			float t_size = float(screen.bounds.h) / distance;

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

			Point texture_origin = mipmap_offset[mipmap_index];
			uint8_t texture_size = mipmap_size[mipmap_index];


			Vec2 uv(
				(float(scanline) / wall_height) + x,
				0.5
			);

			uv.x = uv.x * weight;
			uv.x += x * (1.0f - weight);

			uv.y = uv.y * weight;
			uv.y += z * (1.0f - weight);

			uv.x -= floorf(uv.x);
			uv.y -= floorf(uv.y);

			uint8_t *fragment_c = screen.sprites->ptr(Point(
				texture_origin.x + (uv.x * texture_size),
				texture_origin.y + (uv.y * texture_size)
			));

			screen.pen = screen.sprites->palette[*fragment_c];
			screen.alpha = 200;
			screen.pixel(Point(scanline, ceil_y));
		}

		for (int floor_y = wall_offset_top + wall_height; floor_y < screen.bounds.h; floor_y++) {

			float distance = (float)screen.bounds.h / (2.0f * (floor_y + wall_offset) - screen.bounds.h);
			float weight = distance / z;

			float t_size = float(screen.bounds.h) / distance;

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

			Point texture_origin = mipmap_offset[mipmap_index];
			uint8_t texture_size = mipmap_size[mipmap_index];


			Vec2 uv(
				(float(scanline) / wall_height) + x,
				0.1f
			);

			uv.x = uv.x * weight;
			uv.x += x * (1.0f - weight);

			uv.y = uv.y * weight;
			uv.y += z * (1.0f - weight);

			uv.x -= floorf(uv.x);
			uv.y -= floorf(uv.y);

			uint8_t *fragment_c = screen.sprites->ptr(Point(
				texture_origin.x + (uv.x * texture_size),
				texture_origin.y + (uv.y * texture_size)
			));

			screen.pen = screen.sprites->palette[*fragment_c];
			screen.alpha = 200;
			screen.pixel(Point(scanline, floor_y));

		}
	}

}

void tunnel_test1(uint8_t time_ms) {

	int max_stripe_height = (screen.bounds.h / 3);
	int min_stripe_height = 40;

	for (int scanline = 0; scanline < screen.bounds.w; scanline++) {
		screen.alpha = 255;
		int offset = scanline * 2;
		//float time_offset = (time_ms + offset) / 100.0f;
		float x = (sinf(float(time_ms + offset) / 1000.0f) + 1.0f) / 2.0f;
		float y = (sinf(float(time_ms + offset + 1000) / 2000.0f) + 1.0f) / 2.0f;

		float stripe_height = min_stripe_height + max_stripe_height * y;
		float stripe_offset_top = (screen.bounds.h / 4) * x;

		float t_scale = float(min_stripe_height + max_stripe_height) / stripe_height;

		/*screen.pen = rgba(255, 200, 200);
		screen.line(
			point(scanline, stripe_offset_top),
			point(scanline, stripe_offset_top + stripe_height)
		);*/

		Vec2 c(((scanline * t_scale) + (time_ms / 5.0f)), 0);

		screen.stretch_blit_vspan(screen.sprites, Point(int(c.x) % 128, int(c.y) % 128), 128, Point(scanline, stripe_offset_top), stripe_height);


		for (int f = 0; f < screen.bounds.h - stripe_height - stripe_offset_top - 1; f++) {
			float distance = (float)(screen.bounds.h - stripe_height - stripe_offset_top - 1) / f;

			c.y = distance * stripe_height;
			/*while (c.y < 0) {
				c.y = 127 - c.y;
			}
			while (c.y > 127) {
				c.y -= 127;
			}*/

			c.x -= (scanline - (screen.bounds.w / 2)) / 200.0f;
			/*while (c.x < 0) {
				c.x = 127 - c.x;
			}
			while (c.x > 127) {
				c.x -= 127;
			}*/


			uint8_t *fragment_c = screen.sprites->ptr(int(c.x) % 128, int(c.y) % 128);

			screen.pen = screen.sprites->palette[*fragment_c];
			screen.alpha = 200;
			screen.pixel(Point(scanline, (stripe_offset_top + stripe_height) + f - 1));
		}
	}

}

void render(uint32_t time_ms) {
	uint32_t ms_start = now();

	screen.pen = Pen(0, 0, 0, 255);
	screen.clear();

	tunnel_test(time_ms);

	screen.alpha = 255;

	int x = 20 + (sinf(time_ms / 1000.0f) * 10.0f);
	int y = 40 + (sinf(time_ms / 1500.0f) * 30.0f);

	screen.blit(ss_ship, Rect(0, 0, 64, 32), Point(x, y));

	uint32_t ms_end = now();
	screen.mask = nullptr;
	screen.pen = Pen(255, 0, 0);
	for (uint32_t i = 0; i < (ms_end - ms_start); i++) {
		screen.pen = Pen(i * 5, 255 - (i * 5), 0);
		screen.rectangle(Rect(i * 3 + 1, screen.bounds.h - 3, 2, 2));
	}
}

void update(uint32_t time) {

}

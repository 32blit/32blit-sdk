#include "scrolly-tile.hpp"
#include "graphics/color.hpp"

#define SCREEN_W blit::fb.bounds.w
#define SCREEN_H blit::fb.bounds.h

#define TILE_W 10
#define TILE_H 10
#define TILE_SOLID 0b1 << 15

#define PLAYER_W 2
#define PLAYER_H 4

#define TILES_Y ((SCREEN_H / TILE_H) + 3)
#define TILES_X (SCREEN_W / TILE_W)

#define PLAYER_TOP player_position.y
#define PLAYER_BOTTOM player_position.y + PLAYER_H
#define PLAYER_RIGHT player_position.x + PLAYER_W
#define PLAYER_LEFT player_position.x

#define MAX_JUMP 10

std::vector<uint16_t> tiles;

blit::timer tile_update;
blit::point tile_offset(0, 0);

vec2 player_position(80.0f, SCREEN_H - PLAYER_H);
vec2 player_velocity(0.0f, 0.0f);
vec2 jump_velocity(0.0f, -2.0f);

float water_level = 0;

uint32_t progress = 0;
uint16_t row_mask = 0b1111111001111111;
uint8_t passage_width = 1;

uint16_t last_buttons = 0;
uint32_t jump_pressed = 0;
uint8_t can_jump = 0;
bool can_climb = 0;
uint16_t player_tile_y = 0;

enum e_mask_type {
    UP,
    LEFT,
    RIGHT
};

e_mask_type mask_type = UP;

typedef uint16_t (*tile_callback)(uint16_t tile, uint8_t x, uint8_t y, void *args);

void for_each_tile(tile_callback callback, void *args) {
    for (auto y = 0; y < TILES_Y; y++) {
        for(auto x = 0; x < TILES_X; x++) {
            uint16_t index = (y * TILES_X) + x;
            tiles[index] = callback(tiles[index], x, y, args);
        }    
    }
}

uint16_t get_tile_at(uint8_t x, uint8_t y) {
    if (x < 0) return TILE_SOLID;
    if (x > 15) return TILE_SOLID;
    uint16_t index = (y * TILES_X) + x;
    return tiles[index];
}

#define TILE_LEFT        1 << 7
#define TILE_RIGHT       1 << 6
#define TILE_BELOW       1 << 5
#define TILE_ABOVE       1 << 4
#define TILE_ABOVE_LEFT  1 << 3
#define TILE_ABOVE_RIGHT 1 << 2
#define TILE_BELOW_LEFT  1 << 1
#define TILE_BELOW_RIGHT 1 << 0

uint16_t render_tile(uint16_t tile, uint8_t x, uint8_t y, void *args) {
    blit::point offset = *(blit::point *)args;

    auto tile_x = (x * TILE_W) + offset.x;
    auto tile_y = (y * TILE_H) + offset.y;

    uint8_t feature_map = 0;

    auto tile_top = tile_y;
    auto tile_bottom = tile_y + TILE_H;
    auto tile_left = tile_x;
    auto tile_right = tile_x + TILE_W;

    feature_map |= (get_tile_at(x - 1, y) & TILE_SOLID) ? TILE_LEFT : 0;
    feature_map |= (get_tile_at(x + 1, y) & TILE_SOLID) ? TILE_RIGHT : 0;
    feature_map |= (get_tile_at(x, y - 1) & TILE_SOLID) ? TILE_ABOVE : 0;
    feature_map |= (get_tile_at(x, y + 1) & TILE_SOLID) ? TILE_BELOW : 0;

    feature_map |= (get_tile_at(x - 1, y - 1) & TILE_SOLID) ? TILE_ABOVE_LEFT : 0;
    feature_map |= (get_tile_at(x + 1, y - 1) & TILE_SOLID) ? TILE_ABOVE_RIGHT : 0;
    feature_map |= (get_tile_at(x - 1, y + 1) & TILE_SOLID) ? TILE_BELOW_LEFT : 0;
    feature_map |= (get_tile_at(x + 1, y + 1) & TILE_SOLID) ? TILE_BELOW_RIGHT : 0;

    rgba color_base = blit::hsv_to_rgba(tile_y / 120.0f, 0.5f, 0.8f);
    rgba color_dark = rgba(int(color_base.r * 0.75f), int(color_base.g * 0.75f), int(color_base.b * 0.75f));
    rgba color_darker = rgba(int(color_base.r * 0.5f), int(color_base.g * 0.5f), int(color_base.b * 0.5f));

    if(tile & TILE_SOLID) {
        blit::fb.pen(color_base);
        blit::fb.rectangle(rect(tile_x, tile_y, TILE_W, TILE_H));

        if((PLAYER_RIGHT > tile_left) && (PLAYER_LEFT < tile_right)){
            if(PLAYER_TOP < tile_bottom && PLAYER_BOTTOM > tile_bottom){
                blit::fb.pen(rgba(255, 255, 255));
                blit::fb.rectangle(rect(tile_x, tile_y, TILE_W, TILE_H));
            }
            if((PLAYER_BOTTOM > tile_top) && (PLAYER_TOP < tile_top)){
                blit::fb.pen(rgba(255, 255, 255));
                blit::fb.rectangle(rect(tile_x, tile_y, TILE_W, TILE_H));
            }
        }
        if((PLAYER_BOTTOM > tile_top) && (PLAYER_TOP < tile_bottom)){
            if(PLAYER_LEFT < tile_right && PLAYER_RIGHT > tile_right){
                blit::fb.pen(rgba(255, 255, 255));
                blit::fb.rectangle(rect(tile_x, tile_y, TILE_W, TILE_H));
            }
            if((PLAYER_RIGHT > tile_left) && (PLAYER_LEFT < tile_left)) {
                blit::fb.pen(rgba(255, 255, 255));
                blit::fb.rectangle(rect(tile_x, tile_y, TILE_W, TILE_H));
            }
        }

        if ((feature_map & (TILE_ABOVE_LEFT | TILE_ABOVE | TILE_LEFT)) == 0) {
            blit::fb.pen(rgba(0, 0, 0));
            blit::fb.pixel(point(tile_x, tile_y));
            blit::fb.pen(color_darker);
            blit::fb.pixel(point(tile_x + 1, tile_y));
            blit::fb.pixel(point(tile_x, tile_y + 1));
        }
        if ((feature_map & (TILE_ABOVE_RIGHT | TILE_ABOVE | TILE_RIGHT)) == 0) {
            blit::fb.pen(rgba(0, 0, 0));
            blit::fb.pixel(point(tile_x + TILE_W - 1, tile_y));
            blit::fb.pen(color_darker);
            blit::fb.pixel(point(tile_x + TILE_W - 2, tile_y));
            blit::fb.pixel(point(tile_x + TILE_W - 1, tile_y + 1));
        }
        if ((feature_map & (TILE_BELOW_LEFT | TILE_BELOW | TILE_LEFT)) == 0) {
            blit::fb.pen(rgba(0, 0, 0));
            blit::fb.pixel(point(tile_x, tile_y + TILE_H - 1));
            blit::fb.pen(color_darker);
            blit::fb.pixel(point(tile_x + 1, tile_y + TILE_H - 1));
            blit::fb.pixel(point(tile_x, tile_y + TILE_H - 2));
        }
        if ((feature_map & (TILE_BELOW_RIGHT | TILE_BELOW | TILE_RIGHT)) == 0) {
            blit::fb.pen(rgba(0, 0, 0));
            blit::fb.pixel(point(tile_x + TILE_W - 1, tile_y + TILE_H - 1));
            blit::fb.pen(color_darker);
            blit::fb.pixel(point(tile_x + TILE_W - 2, tile_y + TILE_H - 1));
            blit::fb.pixel(point(tile_x + TILE_W - 1, tile_y + TILE_H - 2));
        }
    } else {
        if(feature_map & TILE_ABOVE) {
            if (feature_map & TILE_LEFT) {
                blit::fb.pen(color_base);
                blit::fb.pixel(point(tile_x, tile_y));
                blit::fb.pen(color_dark);
                blit::fb.pixel(point(tile_x + 1, tile_y));
                blit::fb.pixel(point(tile_x, tile_y + 1));
            }
            if (feature_map & TILE_RIGHT) {
                blit::fb.pen(color_base);
                blit::fb.pixel(point(tile_x + TILE_W - 1, tile_y));
                blit::fb.pen(color_dark);
                blit::fb.pixel(point(tile_x + TILE_W - 2, tile_y));
                blit::fb.pixel(point(tile_x + TILE_W - 1, tile_y + 1));
            }
        }
        if(feature_map & TILE_BELOW) {
            if(feature_map & TILE_LEFT) {
                blit::fb.pen(color_base);
                blit::fb.pixel(point(tile_x, tile_y + TILE_H - 1));
                blit::fb.pen(color_dark);
                blit::fb.pixel(point(tile_x + 1, tile_y + TILE_H - 1));
                blit::fb.pixel(point(tile_x, tile_y + TILE_H - 2));
            }
            if(feature_map & TILE_RIGHT) {
                blit::fb.pen(color_base);
                blit::fb.pixel(point(tile_x + TILE_W - 1, tile_y + TILE_H - 1));
                blit::fb.pen(color_dark);
                blit::fb.pixel(point(tile_x + TILE_W - 2, tile_y + TILE_H - 1));
                blit::fb.pixel(point(tile_x + TILE_W - 1, tile_y + TILE_H - 2));
            }
        }
    }


    return tile;
}

uint8_t count_set_bits(uint16_t number) {
    uint8_t count = 0;
    for(auto x = 0; x < 16; x++){
        if(number & (1 << x)){
            count++;
        }
    }
    return count;
}

void generate_new_row_mask() {
    uint16_t new_row_mask = row_mask;

    uint8_t bits = blit::random() % 4;

    while(count_set_bits(new_row_mask) < 14 - passage_width){
        uint8_t set_bit = blit::random() % 16;
        new_row_mask |= (1 << set_bit);
    }

    uint8_t turning_size = blit::random() % 8;

    switch(blit::random() % 3){
        case 0:
            while(turning_size--){
                new_row_mask &= (new_row_mask >> 1);
                new_row_mask |= 0b1000000000000000;
                new_row_mask &= 0xffff;
            }
            break;
        case 1:
            while(turning_size--){
                new_row_mask &= (new_row_mask << 1);
                new_row_mask |= 0b0000000000000001;
                new_row_mask &= 0xffff;
            }
            break;
    }

    // If our new mask doesn't share are least one unset bit in common
    // with the previous row then the tunnel is not navigable
    // just throw it away
    if(((0xffff - new_row_mask) & (0xffff - row_mask)) == 0){
        new_row_mask = row_mask;
    }

    if(new_row_mask == 0b0111111111111111 && row_mask == 0b0111111111111111){
        new_row_mask = 0b0011111111111111;
    }
    if(new_row_mask == 0b1111111111111110 && row_mask == 0b1111111111111110){
        new_row_mask = 0b1111111111111100;
    }

    /*
    I'd love to blow some random voids in the level to spice
    things up, but this breaks traversal detection and makes
    for levels that can't be completed.

    This should be a texture instead of solid/not solid.
    if (passage_width < 5) {
        new_row_mask &= ~(1 << (blit::random() & 0xf));
    }
    */

    row_mask = new_row_mask;
}

void update_tiles(blit::timer &timer) {
    if (!(player_position.y < 60)) {
        return;
    }
    water_level -= 1;
    player_position.y += 1;
    progress += 1;
    passage_width = int(((sin(progress / 100.0f) + 1.0f)) * 7.0f);
    tile_offset.y += 1;
    if(tile_offset.y >= 0) {
        tile_offset.y = -10;
        //std::rotate(tiles.begin(), tiles.begin() + tiles.size() - TILES_X, tiles.end());

        for(auto row = TILES_Y - 2; row > -1; row--){
            for(auto x = 0; x < TILES_X; x++){
                uint16_t tgt = ((row + 1) * TILES_X) + x;
                uint16_t src = (row * TILES_X) + x;
                tiles[tgt] = tiles[src];
            }
        }

        for(auto x = 0; x < TILES_X; x++) {
            if(row_mask & (1 << x)) {
                tiles[x] = TILE_SOLID;
            }
            else {
                tiles[x] = 0;
            }
        }

        generate_new_row_mask();
    }
}

void init(void) {
    std::srand(239128);
    blit::set_screen_mode(blit::lores);
    tiles.reserve(TILES_X * TILES_Y);

    for(auto y = TILES_Y - 1; y >= 0; y--) {
        for(auto x = 0; x < TILES_X; x++) {
            auto offset = y * TILES_X + x;
            if(row_mask & (1 << x)) {
                tiles[offset] = TILE_SOLID;
            }
            else {
                tiles[offset] = 0;
            }
        }
        generate_new_row_mask();
    }


    tile_update.init(update_tiles, 10, -1);
    tile_update.start();
}


uint16_t collide_player_lr(uint16_t tile, uint8_t x, uint8_t y, void *args) {
    blit::point offset = *(blit::point *)args;

    auto tile_x = (x * TILE_W) + offset.x;
    auto tile_y = (y * TILE_H) + offset.y;

    auto tile_top = tile_y;
    auto tile_bottom = tile_y + TILE_H;
    auto tile_left = tile_x;
    auto tile_right = tile_x + TILE_W;

    if(tile & TILE_SOLID) {
        if((PLAYER_BOTTOM > tile_top) && (PLAYER_TOP < tile_bottom)){
            if(PLAYER_LEFT <= tile_right && PLAYER_RIGHT > tile_right){
                player_position.x = tile_right;
                player_velocity.x = 0.0f;
                jump_velocity.x = 0.5f;
                can_jump = MAX_JUMP;
                can_climb = true;
            }
            if((PLAYER_RIGHT >= tile_left) && (PLAYER_LEFT < tile_left)) {
                player_position.x = tile_left - PLAYER_W;
                player_velocity.x = 0.0f;
                jump_velocity.x = -0.5f;
                can_jump = MAX_JUMP;
                can_climb = true;
            }
        }
    }

    return tile;
}

uint16_t collide_player_ud(uint16_t tile, uint8_t x, uint8_t y, void *args) {
    blit::point offset = *(blit::point *)args;

    auto tile_x = (x * TILE_W) + offset.x;
    auto tile_y = (y * TILE_H) + offset.y;

    auto tile_top = tile_y;
    auto tile_bottom = tile_y + TILE_H;
    auto tile_left = tile_x;
    auto tile_right = tile_x + TILE_W;

    if(tile & TILE_SOLID) {
        if((PLAYER_RIGHT > tile_left) && (PLAYER_LEFT < tile_right)){
            if(PLAYER_TOP < tile_bottom && PLAYER_BOTTOM > tile_bottom){
                player_position.y = tile_bottom;
                player_velocity.y = 0;
            }
            if((PLAYER_BOTTOM > tile_top) && (PLAYER_TOP < tile_top)){
                player_position.y = tile_top - PLAYER_H;
                player_velocity.y = 0;
                can_jump = MAX_JUMP;
            }
        }
    }

    return tile;
}


void update(uint32_t time_ms) {
    water_level += 0.1f;
    uint16_t changed = blit::buttons ^ last_buttons;
    uint16_t pressed = changed & blit::buttons;
    uint16_t released = changed & ~blit::buttons;

    vec2 movement(0, 0);

    if(blit::buttons & blit::button::DPAD_LEFT) {
        player_velocity.x -= 0.1f;
        movement.x = -1;
    }
    if(blit::buttons & blit::button::DPAD_RIGHT) {
        player_velocity.x += 0.1f;
        movement.x = 1;
    }
    if(blit::buttons & blit::button::DPAD_UP) {
        if(can_climb) {
            player_velocity.y -= 0.5;
        }
        movement.y = -1;
    }
    if(blit::buttons & blit::button::DPAD_DOWN) {
        movement.y = 1;
    }

    if(can_climb) {
        player_velocity.y *= 0.5f;
    }


    if(can_jump){
        if(pressed & blit::button::A) {
            player_velocity = jump_velocity;

            can_jump--;
        }
    }

    player_velocity.y += 0.098f;
    player_velocity.y *= 0.99f;
    player_velocity.x *= 0.90f;


    player_position.x += player_velocity.x;
    //player_position.x += movement.x;
    
    jump_velocity.x = 0.0f;
    can_climb = false;
    if(player_position.x <= 0){
        player_position.x = 0;
        can_climb = true;
        can_jump = MAX_JUMP;
        jump_velocity.x = 0.5f;
    }
    if(player_position.x + PLAYER_W >= SCREEN_W) {
        player_position.x = SCREEN_W - PLAYER_W;
        can_climb = true;
        can_jump = MAX_JUMP;
        jump_velocity.x = -0.5f;

    }
    for_each_tile(collide_player_lr, (void *)&tile_offset);


    player_position.y += player_velocity.y;
    //player_position.y += movement.y;

    if(player_position.y + PLAYER_H > SCREEN_H) {
        player_position.y = SCREEN_H - PLAYER_H;
        player_velocity.y = 0;
        can_jump = MAX_JUMP;
    }
    for_each_tile(collide_player_ud, (void *)&tile_offset);

    last_buttons = blit::buttons;
}

void render(uint32_t time_ms) {
    blit::fb.pen(blit::rgba(0, 0, 0));
    blit::fb.clear();
    for_each_tile(render_tile, (void *)&tile_offset);


    blit::fb.pen(blit::rgba(255, 255, 255));
    blit::fb.rectangle(rect(player_position.x, player_position.y, PLAYER_W, PLAYER_H));

    fb.pen(rgba(255, 255, 255));
    fb.text(std::to_string(passage_width), &minimal_font[0][0], point(0, 0));

    if(water_level > 0){
        blit::fb.pen(blit::rgba(100, 100, 255, 100));
        blit::fb.rectangle(rect(0, SCREEN_H - water_level, SCREEN_W, SCREEN_H));
    }
}
#include "scrolly-tile.hpp"
#include "graphics/color.hpp"

#define SCREEN_W blit::fb.bounds.w
#define SCREEN_H blit::fb.bounds.h

#define TILE_W 10
#define TILE_H 10
#define TILE_SOLID 0b1 << 7
#define TILE_WATER 0b1 << 6
#define TILE_LINKED 0b1 << 5

#define WALL_LEFT 1
#define WALL_RIGHT 2
#define WALL_NONE 0

#define PLAYER_W 2
#define PLAYER_H 4

#define TILES_Y uint8_t((SCREEN_H / TILE_H) + 3)
#define TILES_X uint8_t(SCREEN_W / TILE_W)

#define PLAYER_TOP player_position.y
#define PLAYER_BOTTOM player_position.y + PLAYER_H
#define PLAYER_RIGHT player_position.x + PLAYER_W
#define PLAYER_LEFT player_position.x

#define RANDOM_TYPE_HRNG 0
#define RANDOM_TYPE_PRNG 1

#define GAME_STATE_MENU 0
#define GAME_STATE_PLAY 1
#define GAME_STATE_DEAD 2

// Number of times a player can jump sequentially
// including mid-air jumps and the initial ground
// or wall jump
#define MAX_JUMP 3

// All the art is rainbow fun time, so we don't need
// much data about each tile.
// Rounded corners are also procedural depending upon
// tile proximity.
// The screen allows for 16x12 10x10 tiles, but we
// use an extra 3 vertically:
// +1 - because an offset row means we can see 13 rows total
// +2 - because tile features need an adjacent row to generate from
// IE: when our screen is shifted down 5px you can see 13
// rows and both the top and bottom visible row are next to the
// additional two invisible rows which govern how corners are rounded.
uint8_t tiles[16 * 15] = { 0 };

blit::timer state_update;
blit::point tile_offset(0, 0);

vec2 player_position(80.0f, SCREEN_H - PLAYER_H);
vec2 player_velocity(0.0f, 0.0f);
vec2 jump_velocity(0.0f, -2.0f);

float water_level = 0;

uint32_t progress = 0;
uint16_t row_mask = 0xffff;
uint16_t linked_passage_mask = 0;
uint8_t passage_width = 0;
uint8_t last_passage_width = 0;

uint8_t current_random_source = RANDOM_TYPE_PRNG;
uint32_t current_random_seed = 0xf0f0f0f0;

uint8_t passages[] = {
    0,
    0,
    0,
    0,
    0,
};

uint8_t passage_count = 5;

uint16_t last_buttons = 0;
uint32_t jump_pressed = 0;
uint8_t can_jump = 0;
bool can_climb = 0;
bool on_floor = false;
uint8_t climbing_wall = WALL_NONE;
uint16_t player_tile_y = 0;
uint8_t game_state = GAME_STATE_MENU;

typedef uint16_t (*tile_callback)(uint16_t tile, uint8_t x, uint8_t y, void *args);

uint32_t lfsr = 0;
uint16_t tap = 0x74b8;

uint32_t get_random_number() {
    switch(current_random_source) {
        case RANDOM_TYPE_HRNG:
            return blit::random();
        case RANDOM_TYPE_PRNG:
            uint8_t lsb = lfsr & 1;
            lfsr >>= 1;

            if (lsb) {
                lfsr ^= tap;
            }
            return lfsr;
    }
}

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
    if (y > TILES_Y) return 0;
    if(y < 0) return TILE_SOLID;
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
    // Rendering tiles is pretty simple and involves drawing rectangles
    // in the right places.
    // But a large amount of this function is given over to rounding
    // corners depending upon the content of neighbouring tiles.
    // This could probably be rewritten to use a lookup table?
    blit::point offset = *(blit::point *)args;

    auto tile_x = (x * TILE_W) + offset.x;
    auto tile_y = (y * TILE_H) + offset.y;

    uint8_t feature_map = 0;

    feature_map |= (get_tile_at(x - 1, y) & TILE_SOLID) ? TILE_LEFT : 0;
    feature_map |= (get_tile_at(x + 1, y) & TILE_SOLID) ? TILE_RIGHT : 0;
    feature_map |= (get_tile_at(x, y - 1) & TILE_SOLID) ? TILE_ABOVE : 0;
    feature_map |= (get_tile_at(x, y + 1) & TILE_SOLID) ? TILE_BELOW : 0;

    feature_map |= (get_tile_at(x - 1, y - 1) & TILE_SOLID) ? TILE_ABOVE_LEFT : 0;
    feature_map |= (get_tile_at(x + 1, y - 1) & TILE_SOLID) ? TILE_ABOVE_RIGHT : 0;
    feature_map |= (get_tile_at(x - 1, y + 1) & TILE_SOLID) ? TILE_BELOW_LEFT : 0;
    feature_map |= (get_tile_at(x + 1, y + 1) & TILE_SOLID) ? TILE_BELOW_RIGHT : 0;

    bool round_tl = (feature_map & (TILE_ABOVE_LEFT | TILE_ABOVE | TILE_LEFT)) == 0;
    bool round_tr = (feature_map & (TILE_ABOVE_RIGHT | TILE_ABOVE | TILE_RIGHT)) == 0;
    bool round_bl = (feature_map & (TILE_BELOW_LEFT | TILE_BELOW | TILE_LEFT)) == 0;
    bool round_br = (feature_map & (TILE_BELOW_RIGHT | TILE_BELOW | TILE_RIGHT)) == 0;

    rgba color_base = blit::hsv_to_rgba(((120 - tile_y) + 110.0f) / 120.0f, 0.5f, 0.8f);

    if(tile & TILE_SOLID) {
        // Draw tiles without anti-aliasing to save code bloat
        // Uses the rounded corner flags to miss a pixel for a
        // basic rounded corner effect.
        for(auto py = 0; py < TILE_H; py++){
            for(auto px = 0; px < TILE_W; px++){
                if(round_tl && px == 0 && py == 0) continue;
                if(round_tr && px == TILE_W - 1 && py == 0) continue;
                if(round_bl && px == 0 && py == TILE_H - 1) continue;
                if(round_br && px == TILE_H - 1 && py == TILE_H - 1) continue;
                blit::fb.pen(color_base);
                blit::fb.pixel(point(tile_x + px, tile_y + py));
            }
        }
    } else {
        /*
        // Only useful for debugging - lets us see when the generator
        // links orphan passages back to those still in use
        if(tile & TILE_LINKED){
            blit::fb.pen(rgba(100, 100, 100));
            blit::fb.rectangle(rect(tile_x, tile_y, TILE_W, TILE_H));
        }
        */
        /*
        // This might have been a good idea but since we're getting all
        // neighbouring tiles anyway why don't we check for solid walls
        // all around us and fill with water?
        if(tile & TILE_WATER) {
            
            blit::fb.pen(rgba(100, 100, 255, 128));
            blit::fb.rectangle(rect(tile_x, tile_y + 5, TILE_W, 5));
        }
        */
        if(feature_map & TILE_ABOVE) {
            if (feature_map & TILE_LEFT) {
                blit::fb.pen(color_base);
                blit::fb.pixel(point(tile_x, tile_y));
            }
            if (feature_map & TILE_RIGHT) {
                blit::fb.pen(color_base);
                blit::fb.pixel(point(tile_x + TILE_W - 1, tile_y));
            }
        }
        if(feature_map & TILE_BELOW) {
            // If we have a tile directly to the left and right
            // of this one then it's a little pocket we can fill with water!
            if(feature_map & TILE_LEFT && feature_map & TILE_RIGHT) {
                blit::fb.pen(rgba(200, 200, 255, 128));
                blit::fb.rectangle(rect(tile_x, tile_y + (TILE_H / 2), TILE_W, TILE_H / 2));
            }
            if(feature_map & TILE_LEFT) {
                blit::fb.pen(color_base);
                blit::fb.pixel(point(tile_x, tile_y + TILE_H - 1));
            }
            if(feature_map & TILE_RIGHT) {
                blit::fb.pen(color_base);
                blit::fb.pixel(point(tile_x + TILE_W - 1, tile_y + TILE_H - 1));
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
    uint16_t new_row_mask = 0x0000;
    linked_passage_mask = 0;

    // Cut our consistent winding passage through the level
    // by tracking the x coord of our passage we can ensure
    // that it's always navigable without having to reject
    // procedurally generated segments
    for(auto p = 0; p < passage_count; p++){
        if(p > passage_width) {
            continue;
        }
        // Controls how far a passage can snake left/right
        uint8_t turning_size = get_random_number() % 7;

        new_row_mask |= (0x8000 >> passages[p]);

        // At every new generation we choose to branch a passage
        // either left or right, or let it continue upwards.
        switch(get_random_number() % 3){
            case 0: // Passage goes right
                while(turning_size--){
                    if(passages[p] < TILES_X - 1){
                        passages[p] += 1;
                    }
                    new_row_mask |= (0x8000 >> passages[p]);
                }
                break;
            case 1: // Passage goes left
                while(turning_size--){
                    if(passages[p] > 0){
                        passages[p] -= 1;
                    }
                    new_row_mask |= (0x8000 >> passages[p]);
                }
                break;
        }
    }

    // Whenever we have a narrowing of our passage we must check
    // for orphaned passages and link them back to the ones still
    // available, to avoid the player going up a tunnel that ends
    // abruptly :(
    // This routine picks a random passage from the ones remaining
    // and routes every orphaned passage to it.
    if(passage_width < last_passage_width) {
        uint8_t target_passage = 0; //get_random_number() % (passage_width + 1);
        uint8_t target_p_x = passages[target_passage];

        for(auto i = passage_width; i < last_passage_width + 1; i++){
            new_row_mask |= (0x8000 >> passages[i]);
            linked_passage_mask  |= (0x8000 >> passages[i]);

            int8_t direction = (passages[i] < target_p_x) ? 1 : -1;
    
            while(passages[i] != target_p_x) {
                passages[i] += direction;
                new_row_mask |= (0x8000 >> passages[i]);
                linked_passage_mask |= (0x8000 >> passages[i]);
            }
        }
    }
    last_passage_width = passage_width;

    row_mask = ~new_row_mask;
}

void update_tiles() {
    for(auto row = TILES_Y - 2; row > -1; row--){
        for(auto x = 0; x < TILES_X; x++){
            uint16_t tgt = ((row + 1) * TILES_X) + x;
            uint16_t src = (row * TILES_X) + x;
            tiles[tgt] = tiles[src];
        }
    }

    generate_new_row_mask();

    for(auto x = 0; x < TILES_X; x++) {
        if(row_mask & (1 << x)) {
            tiles[x] = TILE_SOLID;
        }
        else {
            tiles[x] = 0;
            if(linked_passage_mask & (1 << x)) {
                tiles[x] |= TILE_LINKED;
            }
        }
    }
}

void update_state(blit::timer &timer) {
    if(game_state != GAME_STATE_DEAD) {
        if (game_state == GAME_STATE_MENU || (game_state == GAME_STATE_PLAY && (player_position.y < 70))) {
            if(water_level > 10){
                water_level -= 1;
            }
            player_position.y += 1;

            progress += 1;
            passage_width = floorf(((sin(progress / 100.0f) + 1.0f) / 2.0f) * passage_count);
            tile_offset.y += 1;

            if(tile_offset.y >= 0) {
                tile_offset.y = -10;
                update_tiles();
            }
        }
    }
}

void place_player() {
    for(auto y = 10; y > 0; y--){
        for(auto x = 0; x < TILES_X; x++){
            uint16_t here = get_tile_at(x, y);
            uint16_t below = get_tile_at(x, y + 1);
            if(below & TILE_SOLID && (here & TILE_SOLID) == 0) {
                player_position.x = (x * TILE_W) + 4;
                player_position.y = (y * TILE_H) + tile_offset.y;
                return;
            }
        }
    }
}

void new_level() {
    lfsr = current_random_seed;

    progress = 0;
    water_level = 0;
    passage_width = floorf(((sin(progress / 100.0f) + 1.0f) / 2.0f) * passage_count);

    for(auto x = 0; x < 5; x++) {
        passages[x] = get_random_number() % 5;
    }

    // Use update_tiles to create the initial game state
    // instead of having a separate loop that breaks in weird ways
    for(auto x = 0; x < TILES_Y; x++) {
        update_tiles();
    }
}

void new_game() {
    new_level();

    player_velocity.x = 0.0f;
    player_velocity.y = 0.0f;
    place_player();

    game_state = GAME_STATE_PLAY;
}

void init(void) {
    blit::set_screen_mode(blit::lores);
    state_update.init(update_state, 10, -1);
    new_level();
    state_update.start();
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
            // Collide the left-hand side of the tile right of player
            if(PLAYER_LEFT <= tile_right && PLAYER_RIGHT > tile_right){
                player_position.x = tile_right;
                player_velocity.x = 0.0f;
                jump_velocity.x = 0.5f;
                if(climbing_wall != WALL_LEFT) {
                    can_jump++;
                }
                can_climb = true;
                climbing_wall = WALL_LEFT;
            }
            // Collide the right-hand side of the tile left of player
            if((PLAYER_RIGHT >= tile_left) && (PLAYER_LEFT < tile_left)) {
                player_position.x = tile_left - PLAYER_W;
                player_velocity.x = 0.0f;
                jump_velocity.x = -0.5f;
                if(climbing_wall != WALL_RIGHT) {
                    can_jump++;
                }
                can_climb = true;
                climbing_wall = WALL_RIGHT;
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
            // Collide the bottom side of the tile above player
            if(PLAYER_TOP < tile_bottom && PLAYER_BOTTOM > tile_bottom){
                player_position.y = tile_bottom;
                player_velocity.y = 0;
                on_floor = false;
            }
            // Collide the top side of the tile below player
            if((PLAYER_BOTTOM > tile_top) && (PLAYER_TOP < tile_top)){
                player_position.y = tile_top - PLAYER_H;
                player_velocity.y = 0;
                can_jump = MAX_JUMP;
                climbing_wall = WALL_NONE;
                on_floor = true;
            }
        }
    }

    return tile;
}


void update(uint32_t time_ms) {
    uint16_t changed = blit::buttons ^ last_buttons;
    uint16_t pressed = changed & blit::buttons;
    uint16_t released = changed & ~blit::buttons;

    if (game_state == GAME_STATE_MENU) {
        if(pressed & blit::button::B) {
            new_game();
        }
        if(pressed & blit::button::DPAD_UP) {
            current_random_source = RANDOM_TYPE_PRNG;
            new_level();
        }
        if(pressed & blit::button::DPAD_DOWN) {
            current_random_source = RANDOM_TYPE_HRNG;
            new_level();
        }
        if(pressed & blit::button::DPAD_RIGHT) {
            if(current_random_source == RANDOM_TYPE_PRNG) {
                current_random_seed++;
                new_level();
            }
        }
        if(pressed & blit::button::DPAD_LEFT) {
            if(current_random_source == RANDOM_TYPE_PRNG) {
                current_random_seed--;
                new_level();
            }
        }
        last_buttons = blit::buttons;
        return;
    }

    if(game_state == GAME_STATE_DEAD){
        if(pressed & blit::button::B) {
            game_state = GAME_STATE_MENU;
        }
        last_buttons = blit::buttons;
        return;
    }

    vec2 movement(0, 0);

    if(game_state == GAME_STATE_PLAY){
        water_level += 0.05f;

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
                on_floor = false;
                can_jump--;
            }
        }
    }

    // Gravity
    player_velocity.y += 0.098f;

    if(on_floor){
        // Ground friction
        player_velocity *= 0.8f;
    }
    else
    {
        // Air friction
        player_velocity.y *= 0.98f;
        player_velocity.x *= 0.95f;
    }

    player_position.x += player_velocity.x;
    // Useful for debug since you can position the player directly
    //player_position.x += movement.x;
    
    if(game_state == GAME_STATE_PLAY) {
        jump_velocity.x = 0.0f;
        can_climb = false;
        if(player_position.x <= 0){
            player_position.x = 0;
            can_climb = true;
            if(climbing_wall != WALL_LEFT) {
                can_jump++;
            }
            climbing_wall = WALL_LEFT;
            jump_velocity.x = 0.5f;
        }
        if(player_position.x + PLAYER_W >= SCREEN_W) {
            player_position.x = SCREEN_W - PLAYER_W;
            can_climb = true;
            if(climbing_wall != WALL_RIGHT) {
                can_jump++;
            }
            climbing_wall = WALL_RIGHT;
            jump_velocity.x = -0.5f;

        }
    }
    for_each_tile(collide_player_lr, (void *)&tile_offset);

    player_position.y += player_velocity.y;
    // Useful for debug since you can position the player directly
    //player_position.y += movement.y;

    for_each_tile(collide_player_ud, (void *)&tile_offset);

    if(player_position.y + PLAYER_H > SCREEN_H) {
        game_state = GAME_STATE_DEAD;
    }
    if(player_position.y > SCREEN_H - water_level) {
        game_state = GAME_STATE_DEAD;
    }

    last_buttons = blit::buttons;
}

void render_summary() {
    std::string text = "Game mode: ";

    if(current_random_source == RANDOM_TYPE_PRNG) {
        text.append("Competitive");
    } else {
        text.append("Random Practice");
    }
    fb.text(text, &minimal_font[0][0], point(10, (SCREEN_H / 2) + 20));

    if(current_random_source == RANDOM_TYPE_PRNG) {
        char buf[9];
        sprintf(buf, "%08X", current_random_seed);
        text = "Level seed: ";
        text.append(buf);
        fb.text(text, &minimal_font[0][0], point(10, (SCREEN_H / 2) + 30));
    }

    text = "Press B";
    fb.text(text, &minimal_font[0][0], point(10, (SCREEN_H / 2) + 40));
}

void render(uint32_t time_ms) {
    blit::fb.pen(blit::rgba(0, 0, 0));
    blit::fb.clear();


    if (game_state == GAME_STATE_MENU) {
        std::string text = "RAINBOW ASCENT";

        for_each_tile(render_tile, (void *)&tile_offset);

        blit::fb.pen(blit::rgba(0, 0, 0, 200));
        blit::fb.clear();

        uint8_t x = 10;
        for(auto c : text) {
            uint8_t y = 20 + (5.0f * sin((time_ms / 250.0f) + (float(x) / text.length() * 2.0f * M_PI)));
            rgba color_letter = blit::hsv_to_rgba((x - 10) / 140.0f, 0.5f, 0.8f);
            blit::fb.pen(color_letter);
            char buf[2];
            buf[0] = c;
            buf[1] = '\0';
            blit::fb.text(buf, &minimal_font[0][0], point(x, y));
            x += 10;
        }
        
        blit::fb.pen(rgba(255, 255, 255, 150));

        render_summary();

        return;
    }

    rgba color_water = blit::hsv_to_rgba(((120 - 120) + 110.0f) / 120.0f, 1.0f, 0.5f);
    color_water.a = 255;

    if(water_level > 0){
        blit::fb.pen(color_water);
        blit::fb.rectangle(rect(0, SCREEN_H - water_level, SCREEN_W, water_level + 1));

        for(auto x = 0; x < SCREEN_W; x++){
            uint16_t offset = x + uint16_t(sin(time_ms / 500.0f) * 5.0f);
            if((offset % 5) > 0){
                blit::fb.pixel(point(x, SCREEN_H - water_level - 1));
            }
            if(((offset + 2) % 5) == 0){
                blit::fb.pixel(point(x, SCREEN_H - water_level - 2));
            }
            if(((offset + 3) % 5) == 0){
                blit::fb.pixel(point(x, SCREEN_H - water_level - 2));
            }
        }
    }

    for_each_tile(render_tile, (void *)&tile_offset);


    // Draw the player
    blit::fb.pen(blit::rgba(255, 255, 255));
    blit::fb.rectangle(rect(player_position.x, player_position.y, PLAYER_W, PLAYER_H));
    blit::fb.pen(blit::rgba(255, 50, 50));
    blit::fb.rectangle(rect(player_position.x, player_position.y, PLAYER_W, 1));

    /*
    // Show number of active passages
    p = std::to_string(passage_width + 1);
    p.append(" passages");
    fb.text(p, &minimal_font[0][0], point(2, 10));
    */

    if(water_level > 0){
        color_water.a = 100;
        blit::fb.pen(color_water);
        blit::fb.rectangle(rect(0, SCREEN_H - water_level, SCREEN_W, water_level + 1));

        for(auto x = 0; x < SCREEN_W; x++){
            uint16_t offset = x + uint16_t(sin(time_ms / 500.0f) * 5.0f);
            if((offset % 5) > 0){
                blit::fb.pixel(point(x, SCREEN_H - water_level - 1));
            }
            if(((offset + 2) % 5) == 0){
                blit::fb.pixel(point(x, SCREEN_H - water_level - 2));
            }
            if(((offset + 3) % 5) == 0){
                blit::fb.pixel(point(x, SCREEN_H - water_level - 2));
            }
        }
    }

    std::string text_height = std::to_string(progress);
    text_height.append("cm");

    if(game_state == GAME_STATE_DEAD) {
        fb.pen(rgba(128, 0, 0, 200));
        fb.rectangle(rect(0, 0, SCREEN_W, SCREEN_H));
        fb.pen(rgba(255, 0, 0, 255));
        fb.text("YOU DIED!", &minimal_font[0][0], point((SCREEN_W / 2) - 20, (SCREEN_H / 2) - 4));

        // Round stats
        fb.pen(rgba(255, 255, 255));

        std::string text = "";

        text = "You climbed: ";
        text.append(text_height);
        fb.text(text, &minimal_font[0][0], point(10, (SCREEN_H / 2) + 10));

        render_summary();
    }
    else
    {
        // Draw the HUD
        fb.pen(rgba(255, 255, 255));
        fb.text(text_height, &minimal_font[0][0], point(2, 2));
    }
}
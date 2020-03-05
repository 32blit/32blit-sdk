#include <cinttypes>
#include "scrolly-tile.hpp"
#include "graphics/color.hpp"

using namespace blit;

#define __AUDIO__

#define SCREEN_W 160
#define SCREEN_H 120

#define TILE_W 10
#define TILE_H 10
#define TILE_SOLID 0b1 << 7
#define TILE_WATER 0b1 << 6

// Bitmask for keeping track of adjacent tiles
// in a single uint8_t
#define TILE_LEFT        1 << 7
#define TILE_RIGHT       1 << 6
#define TILE_BELOW       1 << 5
#define TILE_ABOVE       1 << 4
#define TILE_ABOVE_LEFT  1 << 3
#define TILE_ABOVE_RIGHT 1 << 2
#define TILE_BELOW_LEFT  1 << 1
#define TILE_BELOW_RIGHT 1 << 0

#define PLAYER_W 2
#define PLAYER_H 4

#define TILES_X 16
#define TILES_Y 15

#define PLAYER_TOP (player_position.y)
#define PLAYER_BOTTOM (player_position.y + PLAYER_H)
#define PLAYER_RIGHT (player_position.x + PLAYER_W)
#define PLAYER_LEFT (player_position.x)

#define RANDOM_TYPE_HRNG 0
#define RANDOM_TYPE_PRNG 1

#define PASSAGE_COUNT 5

// Number of times a player can jump sequentially
// including mid-air jumps and the initial ground
// or wall jump
#define MAX_JUMP 3

uint8_t current_random_source = RANDOM_TYPE_PRNG;
uint32_t current_random_seed = 0x64063701;

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

uint32_t current_row = 0;

Timer state_update;
Point tile_offset(0, 0);


Vec2 player_position(80.0f, SCREEN_H - PLAYER_H);
Vec2 player_velocity(0.0f, 0.0f);
Vec2 jump_velocity(0.0f, -2.0f);
uint8_t player_jump_count = 0;
uint32_t player_progress = 0;
bool player_on_floor = false;
enum enum_player_state {
    ground = 0,
    near_wall_left = 1,
    wall_left = 2,
    near_wall_right = 3,
    wall_right = 4,
    air = 5
};
enum_player_state player_state = ground;

uint8_t last_passage_width = 0;

float water_level = 0;

// Used for tracking where the engine has linked a finished passage
// back to those still active.
uint16_t linked_passage_mask = 0;

uint8_t passages[PASSAGE_COUNT] = {0};

// Keep track of game state
enum enum_state {
    menu = 0,
    play = 1,
    dead = 2
};
enum_state game_state = enum_state::menu;

typedef uint8_t (*tile_callback)(uint8_t tile, uint8_t x, uint8_t y, void *args);

uint32_t prng_lfsr = 0;
const uint16_t prng_tap = 0x74b8;

uint32_t get_random_number() {
    switch(current_random_source) {
        default:
            return 0;
        case RANDOM_TYPE_HRNG:
            return blit::random();
        case RANDOM_TYPE_PRNG:
            uint8_t lsb = prng_lfsr & 1;
            prng_lfsr >>= 1;

            if (lsb) {
                prng_lfsr ^= prng_tap;
            }
            return prng_lfsr;
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

uint8_t get_tile_at(uint8_t x, uint8_t y) {
    // Get the tile at a given x/y grid coordinate
    if (x < 0) return TILE_SOLID;
    if (x > 15) return TILE_SOLID;
    if (y > TILES_Y) return 0;
    if(y < 0) return TILE_SOLID;
    uint16_t index = (y * TILES_X) + x;
    return tiles[index];
}

uint8_t get_adjacent_tile_solid_flags(uint8_t x, uint8_t y) {
    // TODO: avoid calls to get_tile_at and use offsets to find
    // adjacent tiles more efficiently.
    uint8_t feature_map = 0;
    feature_map |= (get_tile_at(x - 1, y) & TILE_SOLID) ? TILE_LEFT : 0;
    feature_map |= (get_tile_at(x + 1, y) & TILE_SOLID) ? TILE_RIGHT : 0;
    feature_map |= (get_tile_at(x, y - 1) & TILE_SOLID) ? TILE_ABOVE : 0;
    feature_map |= (get_tile_at(x, y + 1) & TILE_SOLID) ? TILE_BELOW : 0;

    feature_map |= (get_tile_at(x - 1, y - 1) & TILE_SOLID) ? TILE_ABOVE_LEFT : 0;
    feature_map |= (get_tile_at(x + 1, y - 1) & TILE_SOLID) ? TILE_ABOVE_RIGHT : 0;
    feature_map |= (get_tile_at(x - 1, y + 1) & TILE_SOLID) ? TILE_BELOW_LEFT : 0;
    feature_map |= (get_tile_at(x + 1, y + 1) & TILE_SOLID) ? TILE_BELOW_RIGHT : 0;
    return feature_map;
}

uint8_t render_tile(uint8_t tile, uint8_t x, uint8_t y, void *args) {
    // Rendering tiles is pretty simple and involves drawing rectangles
    // in the right places.
    // But a large amount of this function is given over to rounding
    // corners depending upon the content of neighbouring tiles.
    // This could probably be rewritten to use a lookup table?
    Point offset = *(Point *)args;

    auto tile_x = (x * TILE_W) + offset.x;
    auto tile_y = (y * TILE_H) + offset.y;

    uint8_t feature_map = get_adjacent_tile_solid_flags(x, y);

    bool round_tl = (feature_map & (TILE_ABOVE_LEFT | TILE_ABOVE | TILE_LEFT)) == 0;
    bool round_tr = (feature_map & (TILE_ABOVE_RIGHT | TILE_ABOVE | TILE_RIGHT)) == 0;
    bool round_bl = (feature_map & (TILE_BELOW_LEFT | TILE_BELOW | TILE_LEFT)) == 0;
    bool round_br = (feature_map & (TILE_BELOW_RIGHT | TILE_BELOW | TILE_RIGHT)) == 0;

    Pen color_base = hsv_to_rgba(((120 - tile_y) + 110.0f) / 120.0f, 0.5f, 0.8f);

    if(tile & TILE_SOLID) {
        // Draw tiles without anti-aliasing to save code bloat
        // Uses the rounded corner flags to miss a pixel for a
        // basic rounded corner effect.
        for(auto py = 0; py < TILE_H; py++){
            for(auto px = 0; px < TILE_W; px++){
                // Skip drawing the pixels for each rounded corner
                if(round_tl && px == 0 && py == 0) continue;
                if(round_tr && px == TILE_W - 1 && py == 0) continue;
                if(round_bl && px == 0 && py == TILE_H - 1) continue;
                if(round_br && px == TILE_H - 1 && py == TILE_H - 1) continue;
                screen.pen = color_base;
                screen.pixel(Point(tile_x + px, tile_y + py));
            }
        }
    } else {
        if(feature_map & TILE_ABOVE) {
            // Draw the top left/right rounded inside corners
            // for an empty tile.
            if (feature_map & TILE_LEFT) {
                screen.pen = color_base;
                screen.pixel(Point(tile_x, tile_y));
            }
            if (feature_map & TILE_RIGHT) {
                screen.pen = color_base;
                screen.pixel(Point(tile_x + TILE_W - 1, tile_y));
            }
        }
        if(feature_map & TILE_BELOW) {
            // If we have a tile directly to the left and right
            // of this one then it's a little pocket we can fill with water!
            // TODO: Make this not look rubbish
            if(feature_map & TILE_LEFT && feature_map & TILE_RIGHT) {
                screen.pen = Pen(200, 200, 255, 128);
                screen.rectangle(Rect(tile_x, tile_y + (TILE_H / 2), TILE_W, TILE_H / 2));
            }
            // Draw the bottom left/right rounded inside corners
            // for an empty tile.
            if(feature_map & TILE_LEFT) {
                screen.pen = color_base;
                screen.pixel(Point(tile_x, tile_y + TILE_H - 1));
            }
            if(feature_map & TILE_RIGHT) {
                screen.pen = color_base;
                screen.pixel(Point(tile_x + TILE_W - 1, tile_y + TILE_H - 1));
            }
        }
    }

    return tile;
}

uint16_t generate_new_row_mask() {
    uint16_t new_row_mask = 0x0000;
    uint8_t passage_width = floorf(((sinf(current_row / 10.0f) + 1.0f) / 2.0f) * PASSAGE_COUNT);

    // Cut our consistent winding passage through the level
    // by tracking the x coord of our passage we can ensure
    // that it's always navigable without having to reject
    // procedurally generated segments
    for(auto p = 0; p < PASSAGE_COUNT; p++){
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
                    if(passages[p] < TILES_X - 2){
                        passages[p] += 1;
                    }
                    new_row_mask |= (0x8000 >> passages[p]);
                }
                break;
            case 1: // Passage goes left
                while(turning_size--){
                    if(passages[p] > 1){
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
        uint8_t target_passage = get_random_number() % (passage_width + 1);
        uint8_t target_p_x = passages[target_passage];

        for(auto i = passage_width; i < last_passage_width + 1; i++){
            new_row_mask |= (0x8000 >> passages[i]);

            int8_t direction = (passages[i] < target_p_x) ? 1 : -1;
    
            while(passages[i] != target_p_x) {
                passages[i] += direction;
                new_row_mask |= (0x8000 >> passages[i]);
            }
        }
    }
    last_passage_width = passage_width;

    current_row++;
    return ~new_row_mask;
}

void update_tiles() {
    // Shift all of our tile rows down by 1 starting
    // with the second-to-bottom tile which replaces
    // the bottom-most tile.
    for(auto y = TILES_Y - 2; y >= 0; y--){
        for(auto x = 0; x < TILES_X; x++){
            uint16_t tgt = ((y + 1) * TILES_X) + x;
            uint16_t src = (y * TILES_X) + x;
            tiles[tgt] = tiles[src];
        }
    }

    uint16_t row_mask = generate_new_row_mask();

    // Replace the very top row of tiles with our newly
    // generated row mask.
    for(auto x = 0; x < TILES_X; x++) {
        if(row_mask & (1 << x)) {
            tiles[x] = TILE_SOLID;
        }
        else {
            tiles[x] = 0;
        }
    }
}

void update_state(Timer &timer) {
    if (game_state == enum_state::menu) {
        tile_offset.y += 1;
    }

    if (game_state == enum_state::play && (player_position.y < 70)) {
        tile_offset.y += 1;
        if(water_level > 10){
            water_level -= 1;
        }
        player_position.y += 1;
        player_progress += 1;
    }

    if(tile_offset.y >= 0) {
        tile_offset.y = -10;
        update_tiles();
    }
}

bool place_player() {
    // Try to find a suitable place to drop the player
    // where they will be standing on solid ground
    for(auto y = 10; y > 0; y--){
        for(auto x = 0; x < TILES_X; x++){
            uint16_t here = get_tile_at(x, y);
            uint16_t below = get_tile_at(x, y + 1);
            if(below & TILE_SOLID && (here & TILE_SOLID) == 0) {
                player_position.x = (x * TILE_W) + 4;
                player_position.y = (y * TILE_H) + tile_offset.y;
                return true;
            }
        }
    }

    return false;
}

void new_level() {
    prng_lfsr = current_random_seed;

    player_position.x = 80.0f;
    player_position.y = float(SCREEN_H - PLAYER_H);
    player_velocity.x = 0.0f;
    player_velocity.y = 0.0f;
    player_progress = 0;
    tile_offset.y = -10;
    tile_offset.x = 0;
    water_level = 0;
    last_passage_width = 0;
    current_row = 0;

    for(auto x = 0; x < 5; x++) {
        passages[x] = (get_random_number() % 14) + 1;
    }

    // Use update_tiles to create the initial game state
    // instead of having a separate loop that breaks in weird ways
    for(auto y = 0; y < TILES_Y; y++) {
        update_tiles();
    }

    bool placed_successfully = place_player();
}

void new_game() {
    new_level();

    game_state = enum_state::play;
}

void init(void) {
    set_screen_mode(lores);
#ifdef __AUDIO__
    channels[0].waveforms   = Waveform::NOISE;
    channels[0].frequency   = 4200;
    channels[0].attack_ms   = 1;
    channels[0].decay_ms    = 1;
    channels[0].sustain     = 0xffff;
    channels[0].release_ms  = 1;
    channels[0].trigger_attack();

    channels[1].waveforms   = Waveform::SQUARE;
    channels[1].frequency   = 0;
    channels[1].attack_ms   = 30;
    channels[1].decay_ms    = 100;
    channels[1].sustain     = 0;
    channels[1].release_ms  = 0;
#endif
    state_update.init(update_state, 10, -1);
    state_update.start();
    new_level();
}

uint8_t collide_player_lr(uint8_t tile, uint8_t x, uint8_t y, void *args) {
    Point offset = *(Point *)args;

    auto tile_x = (x * TILE_W) + offset.x;
    auto tile_y = (y * TILE_H) + offset.y;

    auto tile_top = tile_y;
    auto tile_bottom = tile_y + TILE_H;
    auto tile_left = tile_x;
    auto tile_right = tile_x + TILE_W;

    if(tile & TILE_SOLID) {
        uint8_t near_wall_distance = 2;
        if(((PLAYER_BOTTOM > tile_top) && (PLAYER_BOTTOM < tile_bottom))
        || ((PLAYER_TOP > tile_top) && PLAYER_TOP < tile_bottom)){
            // Collide the left-hand side of the tile right of player
            if(PLAYER_RIGHT > tile_left && (PLAYER_LEFT < tile_left)){
                // screen.pen = Pen(255, 255, 255, 100);
                // screen.rectangle(rect(tile_x, tile_y, TILE_W, TILE_H));
                player_position.x = float(tile_left - PLAYER_W);
                player_velocity.x = 0.0f;
                player_state = wall_right;
            }
            else if(((PLAYER_RIGHT + near_wall_distance) > tile_left) && (PLAYER_LEFT < tile_left)) {
                player_state = near_wall_right;
            }
            // Collide the right-hand side of the tile left of player
            if((PLAYER_LEFT < tile_right) && (PLAYER_RIGHT > tile_right)) {
                // screen.pen = Pen(255, 255, 255, 100);
                // screen.rectangle(rect(tile_x, tile_y, TILE_W, TILE_H));
                player_position.x = float(tile_right);
                player_velocity.x = 0.0f;
                player_state = wall_left;
            }
            else if(((PLAYER_LEFT - near_wall_distance) < tile_right) && (PLAYER_RIGHT > tile_right)) {
                player_state = near_wall_left;
            }
        }
    }

    return tile;
}

uint8_t collide_player_ud(uint8_t tile, uint8_t x, uint8_t y, void *args) {
    Point offset = *(Point *)args;

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
                player_position.y = float(tile_bottom);
                player_velocity.y = 0;
            }
            // Collide the top side of the tile below player
            if((PLAYER_BOTTOM > tile_top) && (PLAYER_TOP < tile_top)){
                player_position.y = float(tile_top - PLAYER_H);
                player_velocity.y = 0;
                player_jump_count = MAX_JUMP;
                player_state = ground;
            }
        }
    }

    return tile;
}

void update(uint32_t time_ms) {
    static uint16_t last_buttons = 0;
    uint16_t changed = buttons ^ last_buttons;
    uint16_t pressed = changed & buttons;
    uint16_t released = changed & ~buttons;

    int32_t water_dist = player_position.y - (SCREEN_H - water_level);
    if (water_dist < 0) {
        water_dist = 0;
    }
#ifdef __AUDIO__
    channels[0].volume      = 4000 + (sinf(float(time_ms) / 1000.0f) * 3000);
#endif

    if (game_state == enum_state::menu) {
        if(pressed & Button::B) {
            new_game();
        }
        else if(pressed & Button::DPAD_UP) {
            current_random_source = RANDOM_TYPE_PRNG;
            new_level();
        }
        else if(pressed & Button::DPAD_DOWN) {
            current_random_source = RANDOM_TYPE_HRNG;
            new_level();
        }
        else if(pressed & Button::DPAD_RIGHT) {
            if(current_random_source == RANDOM_TYPE_PRNG) {
                current_random_seed++;
                new_level();
            }
        }
        else if(pressed & Button::DPAD_LEFT) {
            if(current_random_source == RANDOM_TYPE_PRNG) {
                current_random_seed--;
                new_level();
            }
        }
        last_buttons = buttons;
        return;
    }

    if(game_state == enum_state::dead){
        if(pressed & Button::B) {
            game_state = enum_state::menu;
        }
        last_buttons = buttons;
        return;
    }

    if(game_state == enum_state::play){
        static enum_player_state last_wall_jump = enum_player_state::ground;
#ifdef __AUDIO__
        static float jump_sweep = 0.0;
#endif

        Vec2 movement(0, 0);
        water_level += 0.05f;
        jump_velocity.x = 0.0f;

        // Apply Gravity
        player_velocity.y += 0.098f;

        if(buttons & Button::DPAD_LEFT) {
            player_velocity.x -= 0.1f;
            movement.x = -1;

            if(buttons & Button::DPAD_UP) {
                if(player_state == wall_left
                || player_state == near_wall_left) {
                    player_velocity.y -= 0.12f;
                }
                movement.y = -1;
            }
        }
        if(buttons & Button::DPAD_RIGHT) {
            player_velocity.x += 0.1f;
            movement.x = 1;

            if(buttons & Button::DPAD_UP) {
                if(player_state == wall_right
                || player_state == near_wall_right) {
                    player_velocity.y -= 0.12f;
                }
                movement.y = -1;
            }
        }
        if(buttons & Button::DPAD_DOWN) {
            movement.y = 1;
        }

        if(player_jump_count){
            if(pressed & Button::A) {
                if(player_state == wall_left
                || player_state == wall_right
                || player_state == near_wall_left
                || player_state == near_wall_right) {
                    enum_player_state wall_jump_state = (player_state == wall_left || player_state == near_wall_left) ? wall_left : wall_right;
                    jump_velocity.x = (wall_jump_state == wall_left) ? 1.2f : -1.2f;
                    if(last_wall_jump != wall_jump_state) {
                        player_jump_count = MAX_JUMP;
                    }
                    last_wall_jump = wall_jump_state;
                }
                player_velocity = jump_velocity;
                player_state = air;
                player_jump_count--;
#ifdef __AUDIO__
                channels[1].trigger_attack();
                jump_sweep = 1.0f;
#endif
            }
        }
#ifdef __AUDIO__
        if(jump_sweep > 0) {
            channels[1].frequency = 880 - (880.0f * jump_sweep);
            jump_sweep -= 0.05f;
        }
#endif

        switch(player_state) {
            case wall_left:
            case wall_right:
            case near_wall_left:
            case near_wall_right:
                if ((buttons & Button::DPAD_LEFT) && (player_state == wall_left || player_state == near_wall_left)){
                    player_velocity.y *= 0.5f;
                    break;
                }
                if ((buttons & Button::DPAD_RIGHT) && (player_state == wall_right || player_state == near_wall_right)){
                    player_velocity.y *= 0.5f;
                    break;
                }
                // Fall through to air friction
            case air:
                // Air friction
                player_velocity.y *= 0.98f;
                player_velocity.x *= 0.91f;
                break;
            case ground:
                // Ground friction
                player_velocity *= 0.8f;
                break;
        }

        // Default state is in the air unless we collide
        // with a wall or the ground
        player_state = enum_player_state::air;

        player_position.x += player_velocity.x;
        // Useful for debug since you can position the player directly
        //player_position.x += movement.x;

        if(player_position.x <= 0){
            player_position.x = 0;
            player_velocity.x = 0;
            player_state = wall_left;
        }
        else if(player_position.x + PLAYER_W >= SCREEN_W) {
            player_position.x = float(SCREEN_W - PLAYER_W);
            player_velocity.x = 0;
            player_state = wall_right;

        }
        for_each_tile(collide_player_lr, (void *)&tile_offset);

        player_position.y += player_velocity.y;
        // Useful for debug since you can position the player directly
        //player_position.y += movement.y;

        if(player_position.y + PLAYER_H > SCREEN_H) {
            game_state = enum_state::dead;
        } else if(player_position.y > SCREEN_H - water_level) {
            game_state = enum_state::dead;
        }
        for_each_tile(collide_player_ud, (void *)&tile_offset);
    }

    last_buttons = buttons;
}

void render_summary() {
    std::string text = "Game mode: ";

    if(current_random_source == RANDOM_TYPE_PRNG) {
        text.append("Competitive");
    } else {
        text.append("Random Practice");
    }
    screen.text(text, minimal_font, Point(10, (SCREEN_H / 2) + 20));

    if(current_random_source == RANDOM_TYPE_PRNG) {
        char buf[9];
        snprintf(buf, 9, "%08" PRIX32, current_random_seed);
        text = "Level seed: ";
        text.append(buf);
        screen.text(text, minimal_font, Point(10, (SCREEN_H / 2) + 30));
    }

    text = "Press B";
    screen.text(text, minimal_font, Point(10, (SCREEN_H / 2) + 40));
}

void render(uint32_t time_ms) {
    screen.pen = Pen(0, 0, 0);
    screen.clear();
    std::string text = "RAINBOW ASCENT";

    if (game_state == enum_state::menu) {
        for_each_tile(render_tile, (void *)&tile_offset);

        // Draw the player
        screen.pen = Pen(255, 255, 255);
        screen.rectangle(Rect(player_position.x, player_position.y, PLAYER_W, PLAYER_H));
        screen.pen = Pen(255, 50, 50);
        screen.rectangle(Rect(player_position.x, player_position.y, PLAYER_W, 1));

        screen.pen = Pen(0, 0, 0, 200);
        screen.clear();

        uint8_t x = 10;
        for(auto c : text) {
            uint8_t y = 20 + (5.0f * sinf((time_ms / 250.0f) + (float(x) / text.length() * 2.0f * pi)));
            Pen color_letter = hsv_to_rgba((x - 10) / 140.0f, 0.5f, 0.8f);
            screen.pen = color_letter;
            char buf[2];
            buf[0] = c;
            buf[1] = '\0';
            screen.text(buf, minimal_font, Point(x, y));
            x += 10;
        }

        screen.pen = Pen(255, 255, 255, 150);

        render_summary();

        return;
    }

    Pen color_water = hsv_to_rgba(((120 - 120) + 110.0f) / 120.0f, 1.0f, 0.5f);
    color_water.a = 255;

    if(water_level > 0){
        screen.pen = color_water;
        screen.rectangle(Rect(0, SCREEN_H - water_level, SCREEN_W, water_level + 1));

        for(auto x = 0; x < SCREEN_W; x++){
            uint16_t offset = x + uint16_t(sinf(time_ms / 500.0f) * 5.0f);
            if((offset % 5) > 0){
                screen.pixel(Point(x, SCREEN_H - water_level - 1));
            }
            if(((offset + 2) % 5) == 0){
                screen.pixel(Point(x, SCREEN_H - water_level - 2));
            }
            if(((offset + 3) % 5) == 0){
                screen.pixel(Point(x, SCREEN_H - water_level - 2));
            }
        }
    }

    for_each_tile(render_tile, (void *)&tile_offset);

    // Draw the player
    screen.pen = Pen(255, 255, 255);
    screen.rectangle(Rect(player_position.x, player_position.y, PLAYER_W, PLAYER_H));
    screen.pen = Pen(255, 50, 50);
    screen.rectangle(Rect(player_position.x, player_position.y, PLAYER_W, 1));

    /*
    // Show number of active passages
    p = std::to_string(passage_width + 1);
    p.append(" passages");
    screen.text(p, minimal_font, point(2, 10));
    */

    if(water_level > 0){
        color_water.a = 100;
        screen.pen = color_water;
        screen.rectangle(Rect(0, SCREEN_H - water_level, SCREEN_W, water_level + 1));

        for(auto x = 0; x < SCREEN_W; x++){
            uint16_t offset = x + uint16_t(sinf(time_ms / 500.0f) * 5.0f);
            if((offset % 5) > 0){
                screen.pixel(Point(x, SCREEN_H - water_level - 1));
            }
            if(((offset + 2) % 5) == 0){
                screen.pixel(Point(x, SCREEN_H - water_level - 2));
            }
            if(((offset + 3) % 5) == 0){
                screen.pixel(Point(x, SCREEN_H - water_level - 2));
            }
        }
    }

    if(game_state == enum_state::dead) {
        screen.pen = Pen(128, 0, 0, 200);
        screen.rectangle(Rect(0, 0, SCREEN_W, SCREEN_H));
        screen.pen = Pen(255, 0, 0, 255);
        screen.text("YOU DIED!", minimal_font, Point((SCREEN_W / 2) - 20, (SCREEN_H / 2) - 4));

        // Round stats
        screen.pen = Pen(255, 255, 255);

        std::string text = "";

        text = "You climbed: ";
        text.append(std::to_string(player_progress));
        text.append("cm");
        screen.text(text, minimal_font, Point(10, (SCREEN_H / 2) + 10));

        render_summary();
    }
    else
    {
        // Draw the HUD
        screen.pen = Pen(255, 255, 255);

        text = std::to_string(player_progress);
        text.append("cm");
        screen.text(text, minimal_font, Point(2, 2));

        /*
        // State debug info
        text = "Jumps: ";
        text.append(std::to_string(player_jump_count));
        screen.text(text, minimal_font, point(2, 12));

        text = "State: ";
        switch(player_state){
            case enum_player_state::ground:
                text.append("GROUND");
                break;
            case enum_player_state::air:
                text.append("AIR");
                break;
            case enum_player_state::near_wall_left:
                text.append("NEAR L");
                break;
            case enum_player_state::wall_left:
                text.append("WALL L");
                break;
            case enum_player_state::near_wall_right:
                text.append("NEAR R");
                break;
            case enum_player_state::wall_right:
                text.append("WALL R");
                break;
        }
        screen.text(text, minimal_font, point(2, 22));
        */
    }
}

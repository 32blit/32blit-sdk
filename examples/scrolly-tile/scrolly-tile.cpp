#include "scrolly-tile.hpp"

#define W blit::fb.bounds.w
#define H blit::fb.bounds.h

#define TILE_W 10
#define TILE_H 10
#define TILE_SOLID 0b1 << 15

#define TILES_Y ((H / TILE_H) + 1)
#define TILES_X (W / TILE_W)

std::vector<uint16_t> tiles;

blit::timer tile_update;
blit::point tile_offset(0, 0);

uint16_t row_mask = 0b1111111001111111;

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

uint16_t render_tile(uint16_t tile, uint8_t x, uint8_t y, void *args) {
    blit::point offset = *(blit::point *)args;

    if(tile & TILE_SOLID) {
        switch(tile & 0b111){
            case 0:
                blit::fb.pen(rgba(200, 128, 128));
                break;
            case 1:
                blit::fb.pen(rgba(128, 200, 128));
                break;
            case 2:
                blit::fb.pen(rgba(128, 128, 200));
                break;
            default:
                blit::fb.pen(rgba(128, 128, 128));
        }
        blit::fb.rectangle(rect((x * TILE_W) + offset.x, (y * TILE_H) + offset.y, TILE_W, TILE_H));
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

    uint8_t bits = rand() % 4;

    while(count_set_bits(new_row_mask) < 14){
        uint8_t set_bit = std::rand() % 16;
        new_row_mask |= (1 << set_bit);
    }

    uint8_t turning_size = std::rand() % 8;

    switch(std::rand() % 3){
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

    row_mask = new_row_mask;
}

void update_tiles(blit::timer &timer) {
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


    tile_update.init(update_tiles, 250, -1);
    tile_update.start();
}



void update(uint32_t time_ms) {
}

void render(uint32_t time_ms) {
    blit::fb.pen(blit::rgba(0, 0, 0));
    blit::fb.clear();
    for_each_tile(render_tile, (void *)&tile_offset);
}
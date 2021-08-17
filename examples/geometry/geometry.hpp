#include "32blit.hpp"

#define __AUDIO__
//#define __DEBUG__


const int16_t STARTING_ENERGY = 500;
const int16_t REGEN_ENERGY = 1;
const int16_t LASER_COST = 2;
const int16_t MOVEMENT_COST = 1;
const int16_t DAMAGE_COST = 10;
const uint8_t STARTING_LIVES = 3;
const uint8_t P_MAX_AGE = 255;

const uint8_t ASTEROID_COUNT = 5;
const uint8_t ASTEROID_MIN_R = 40;
const uint8_t ASTEROID_MAX_R = 50;

const uint16_t ASTEROID_MIN_AREA = 100;


struct SpaceDust {
    blit::Vec2 pos;
    blit::Vec2 vel;
    uint32_t age = 0;
    uint8_t color = 0;

    SpaceDust(blit::Vec2 pos, blit::Vec2 vel, uint8_t color) : pos(pos), vel(vel), color(color) {};
};

struct Player {
    blit::Vec2 velocity;
    blit::Vec2 position;
    float rotation = 0;
    float rotational_velocity = 0;
    std::vector<blit::Vec2> shape;
    int32_t energy = 0;
    unsigned int score = 0;
    bool shot_fired = false;
    unsigned int t_shot_fired = 0;
    blit::Vec2 shot_origin;
    blit::Vec2 shot_target;
    int32_t shot_charge;
    uint8_t lives = 3;
    bool invincible = false;

    void reset_or_die() {
        energy = STARTING_ENERGY;
        if(lives == 0) {
            lives = STARTING_LIVES;
            score = 0;
        }
        invincible = true;
        position = blit::Vec2(blit::screen.bounds.w / 2, blit::screen.bounds.h / 2);
        rotation = 0.0f;
        rotational_velocity = 0.0f;
    }
};

struct Polygon {
    float colour_offset;
    blit::Vec2 velocity;
    float rotational_velocity = 0;
    blit::Vec2 origin;
    std::vector<blit::Vec2> points;
    bool prune = false;
    uint16_t area = 0;
};
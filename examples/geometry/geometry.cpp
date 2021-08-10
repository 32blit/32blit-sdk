#include "geometry.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"
#include "engine/particle.hpp"

using namespace blit;

#define __AUDIO__
//#define __DEBUG__


const int16_t STARTING_ENERGY = 500;
const int16_t REGEN_ENERGY = 1;
const int16_t LASER_COST = 50;
const int16_t MOVEMENT_COST = 3;
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

    SpaceDust(Vec2 pos, Vec2 vel, uint8_t color) : pos(pos), vel(vel), color(color) {};
};

struct player {
    Vec2 velocity;
    Vec2 position;
    float rotation = 0;
    float rotational_velocity = 0;
    std::vector<Vec2> shape;
    int32_t energy = 0;
    unsigned int score = 0;
    bool shot_fired = false;
    unsigned int t_shot_fired = 0;
    Vec2 shot_origin;
    Vec2 shot_target;
    uint8_t lives = 3;
    bool invincible = false;

    void reset_or_die() {
        energy = STARTING_ENERGY;
        if(lives == 0) {
            lives = STARTING_LIVES;
            score = 0;
        }
        invincible = true;
        position = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
        rotation = 0.0f;
        rotational_velocity = 0.0f;
    }
};

struct polygon {
    float colour_offset;
    Vec2 velocity;
    float rotational_velocity = 0;
    Vec2 origin;
    std::vector<Vec2> points;
    bool prune = false;
    uint16_t area = 0;
};

std::vector<SpaceDust> particles;
std::vector<polygon> polygons;

player player1;

void explode(Vec2 origin, float factor=1.0f) {
    channels[2].frequency = 800;
    channels[2].trigger_attack();
    uint8_t count = 5 + (blit::random() % 25);
    count *= factor;
    for(auto x = 0u; x < count; x++) {
        float r = blit::random() % 360;
        float v = (50.0f + (blit::random() % 100)) / 100.0f;
        float c = 100 + blit::random() % 128;
        r  = r * pi / 180.0f;

        particles.push_back(SpaceDust(
                origin,
                Vec2(cosf(r) * v, sinf(r) * v),
                c
        ));
    }
}

void thrust(Vec2 origin, Vec2 vel) {
    vel.rotate(pi);
    for(auto x = 0u; x < 10; x++) {
        float r = ((blit::random() % 300) - 150)  / 5000.0f;
        vel.rotate(r);
        particles.push_back(SpaceDust(
            origin,
            vel,
            255
        ));
    }
}

bool prune_polygons(polygon p){
    if(p.prune) {
        explode(p.origin, p.area / float(ASTEROID_MIN_AREA));
    }
    return p.prune;
}

uint16_t area_of_polygon(std::vector<Vec2> &points) {
    auto a = points.back();
    uint16_t area = 0;
    for (auto b : points) {
        area += ((a.x * b.y) - (b.x * a.y)) / 2u;
        a = b;
    }
    // Fix for infinitely small poygons shooting to 65535
    return area > 60000 ? 0 : area;
}

Vec2 centroid_of_polygon(std::vector<Vec2> &points) {
    Vec2 t(0, 0);
    for (auto p : points) {
        t += p;
    }
    return t / points.size();
}

bool line_segment_intersection(Vec2 *intersection, Vec2 a, Vec2 b, Vec2 c, Vec2 d) {
    Vec2 r = b - a;
    Vec2 s = d - c;

    float rxs = (r.x * s.y) - (r.y * s.x);
    float u = ((c.x - a.x) * r.y - (c.y - a.y) * r.x) / rxs;
    float t = ((c.x - a.x) * s.y - (c.y - a.y) * s.x) / rxs;

    if (0 <= u && u <= 1 && 0 <= t && t <= 1) {
        *intersection = a + (r * t);
        return true;
    }

    return false;
}

polygon split_polygon(polygon *poly, Vec2 a, Vec2 b) {
    std::vector<Vec2> split_a;
    std::vector<Vec2> split_b;
    std::vector<Vec2> split_line_segment;

    Vec2 last_point = poly->points.back();

    for (auto &p : poly->points) {
        float bside = ((p.x - a.x) * (b.y - a.y)) - ((p.y - a.y) * (b.x - a.x));
        float last_bside = ((last_point.x - a.x) * (b.y - a.y)) - ((last_point.y - a.y) * (b.x - a.x));

        // If our polygon line croses out intersection line, create a new point along the intersection for each new polygon
        if (bside != last_bside) {
            Vec2 intersection;
            if (line_segment_intersection(&intersection, last_point, p, a, b)) {
                split_a.emplace_back(intersection);
                split_b.emplace_back(intersection);
                split_line_segment.emplace_back(intersection);
                explode(Vec2(intersection.x, intersection.y), 0.25f);
            }
        }

        if (bside > 0) {
            split_a.push_back(Vec2(p.x, p.y));
        }
        else {
            split_b.push_back(Vec2(p.x, p.y));
        }

        last_point = p;
    }

    // If we don't encounter two line segment intersections, we're shooting from *inside* an asteroid
    if (split_b.size() > 0 && split_line_segment.size() == 2) {

        poly->points = std::vector<Vec2>(split_a);
        poly->origin = centroid_of_polygon(poly->points);
        //poly->rotational_velocity += 0.005f;
        poly->area = area_of_polygon(poly->points);

        if(poly->area < ASTEROID_MIN_AREA) {
            player1.score += poly->area;
            poly->prune = true;
        } else {
            poly->prune = false;
        }

        float area = area_of_polygon(split_b);
        Vec2 midpoint = split_line_segment.front() + split_line_segment.back();
        midpoint /= 2.0f;
        Vec2 original_velocity = poly->velocity;

        poly->velocity = poly->origin - midpoint;
        poly->velocity.normalize();
        poly->velocity *= 0.1f;
        poly->velocity += original_velocity;
        poly->rotational_velocity = poly->origin.angle(midpoint) * 0.1f;

        if(area >= ASTEROID_MIN_AREA) {

            polygon new_polygon;
            new_polygon.colour_offset = poly->colour_offset;
            new_polygon.points = std::vector<Vec2>(split_b);
            new_polygon.origin = centroid_of_polygon(new_polygon.points);

            // Push the new asteroid half away from the "cut"
            new_polygon.velocity = new_polygon.origin - midpoint;
            new_polygon.velocity.normalize();
            new_polygon.velocity *= 0.1f;
            new_polygon.velocity += original_velocity;

            new_polygon.rotational_velocity = new_polygon.origin.angle(midpoint) * 0.1f;

            new_polygon.area = area;
            return new_polygon;
        }
        else {
            player1.score += area;
            // This polygon doesn't ever exist, but since we have the list
            // of points and its bounds we can emenate an explosion from its center
            explode(centroid_of_polygon(split_b), area / float(ASTEROID_MIN_AREA));
        }
    }
    
    polygon new_polygon;
    return new_polygon;
}

void draw_polygon(std::vector<Vec2> &points) {
    Vec2 last_point = points.back();
    for (auto &p : points) {
        screen.line(last_point, p);
        last_point = p;
    }
}

std::vector<Vec2> random_convex_polygon(Vec2 origin, float radius) {
    unsigned int count = (rand() % 7) + 3;
    origin += Vec2(radius, radius);
    std::vector<float> angles;
    for (auto a = 0u; a < count; a++) {
        angles.push_back(float(rand() % 360) * pi / (float)180);
    }
    std::sort(angles.begin(), angles.end());
    std::vector<Vec2> points;
    for (auto &angle : angles) {
        Vec2 p = Vec2(0, -radius);
        p.rotate(angle);
        points.push_back(p + origin);
    }
    return points;
}

void rotate_polygon(std::vector<Vec2> &points, float angle, Vec2 origin) {
    Mat3 t = Mat3::identity();
    t *= Mat3::translation(origin);
    t *= Mat3::rotation(angle);
    t *= Mat3::translation(-origin);
    for (auto &p : points) {
        p *= t;
    }
}

void translate_polygon(polygon &polygon, Vec2 translation) {
    Mat3 t = Mat3::identity();
    t *= Mat3::translation(translation);
    for (auto &p : polygon.points) {
        p *= t;
    }
    polygon.origin *= t;
}

void translate_polygon(std::vector<Vec2> &points, Vec2 translation) {
    Mat3 t = Mat3::identity();
    t *= Mat3::translation(translation);
    for (auto &p : points) {
        p *= t;
    }
}

void init() {
    set_screen_mode(ScreenMode::hires);
    for(unsigned int i = 0; i < ASTEROID_COUNT; i++){
        polygon p;
        float x = rand() % screen.bounds.w;
        float y = rand() % screen.bounds.h;
        //float x = screen.bounds.w / 2;
        //float y = screen.bounds.h / 2;
        float r = (rand() % (ASTEROID_MAX_R - ASTEROID_MIN_R)) + ASTEROID_MIN_R;
        float vx = ((rand() % 10) - 5) / 25.0f;
        float vy = ((rand() % 10) - 5) / 25.0f;
        p.colour_offset = (rand() % 100) / 100.0f;
        p.points = random_convex_polygon(Vec2(x, y), r);
        p.velocity = Vec2(vx, vy);
        p.origin = centroid_of_polygon(p.points);
        p.rotational_velocity = ((rand() % 10) - 5) / 1000.0f;
        p.area = area_of_polygon(p.points);
        polygons.push_back(p);
    }

    player1.reset_or_die();
    player1.shape.push_back(Vec2(0, -6));
    player1.shape.push_back(Vec2(-6, 6));
    player1.shape.push_back(Vec2(0, 2));
    player1.shape.push_back(Vec2(6, 6));

#ifdef __AUDIO__
    channels[0].waveforms   = Waveform::NOISE;
    channels[0].frequency   = 4200;
    channels[0].attack_ms   = 10;
    channels[0].decay_ms    = 1;
    channels[0].sustain     = 0xffff;
    channels[0].release_ms  = 10;
    channels[0].volume      = 4000;

    channels[1].waveforms   = Waveform::SINE;
    channels[1].frequency   = 0;
    channels[1].attack_ms   = 10;
    channels[1].decay_ms    = 500;
    channels[1].sustain     = 0;
    channels[1].release_ms  = 0;
    channels[1].volume      = 3000;

    channels[2].waveforms   = Waveform::NOISE;
    channels[2].frequency   = 800;
    channels[2].attack_ms   = 10;
    channels[2].decay_ms    = 500;
    channels[2].sustain     = 0;
    channels[2].release_ms  = 0;
    channels[2].volume      = 8000;
#endif
}

void render(uint32_t time) {
#ifdef __DEBUG__
    uint32_t ms_start = now();
#endif
    float h = time / (pi * 2) / 100.0f;

    screen.pen = Pen(0, 0, 0);
    screen.clear();

    if(player1.invincible) {
        uint8_t rgb = (sinf(time / 200.0f) * 70) + 100;
        screen.pen = Pen(rgb, rgb, rgb);
    }
    else {
        screen.pen = Pen(255, 255, 255);
    }
    std::vector<Vec2> player1_shape(player1.shape);
    translate_polygon(player1_shape, player1.position);
    rotate_polygon(player1_shape, player1.rotation, player1.position);
    draw_polygon(player1_shape);

    for(auto &p: polygons){
        Pen c = hsv_to_rgba(h / (pi * 2) + p.colour_offset, 1.0, 1.0);
        screen.pen = c;
        draw_polygon(p.points);
#ifdef __DEBUG__
        screen.text(std::to_string(p.area), minimal_font, Point(p.origin), true, center_center);
        screen.pen = Pen(255, 255, 255);
        screen.pixel(p.origin);
#endif
    }

    for(auto &p: particles){
        screen.pen = Pen(p.color, p.color, p.color, P_MAX_AGE - (uint8_t)p.age);
        p.age += 2;
        p.pos += p.vel;
        screen.pixel(p.pos);
    }

    particles.erase(std::remove_if(particles.begin(), particles.end(), [](SpaceDust particle) { return (particle.age >= P_MAX_AGE); }), particles.end());


    if(time - player1.t_shot_fired > 0 && time - player1.t_shot_fired < 500){
        int c = 255 - ((time - player1.t_shot_fired ) / 2);
        screen.pen = Pen(c, c, c);
        screen.line(player1.shot_origin, player1.shot_target);
    }

#ifdef __DEBUG__
    uint32_t ms_end = now();

    // draw FPS meter
    screen.alpha = 255;
    screen.pen = Pen(255, 255, 255, 100);
    screen.rectangle(Rect(1, screen.bounds.h - 10, 12, 9));
    screen.pen = Pen(255, 255, 255, 200);
    std::string fms = std::to_string(ms_end - ms_start);
    screen.text(fms, minimal_font, Rect(3, screen.bounds.h - 9, 10, 16));

    int block_size = 4;
    for (uint32_t i = 0; i < (ms_end - ms_start); i++) {
      screen.pen = Pen(i * 5, 255 - (i * 5), 0);
      screen.rectangle(Rect(i * (block_size + 1) + 1 + 13, screen.bounds.h - block_size - 1, block_size, block_size));
    }
#endif

    screen.pen = Pen(255, 255, 255);
    screen.text("score: " + std::to_string(player1.score) + " lives: " + std::to_string(player1.lives), minimal_font, Point(5, 5));

    screen.pen = Pen(0, 255, 0, 200);
    Rect energy = Rect(5, screen.bounds.h - 5, 0, 5);
    energy.y -= energy.h;
    energy.w = player1.energy * (screen.bounds.w - 10) / STARTING_ENERGY;
    screen.rectangle(energy);
}

void update(uint32_t time) {

    Vec2 movement(0, 0);

    if (pressed(Button::DPAD_LEFT))  { player1.rotational_velocity += pi / 720; }
    if (pressed(Button::DPAD_RIGHT)) { player1.rotational_velocity -= pi / 720; }
    //player1.rotational_velocity -= joystick.x * pi / 720;

    if(player1.energy >= MOVEMENT_COST) {
        if (pressed(Button::DPAD_UP))    { movement.y -= 0.03f; }
        if (pressed(Button::DPAD_DOWN))  { movement.y += 0.03f; }
        //movement.y += joystick.y / 10.0f;

        if(pressed(Button::DPAD_UP) || pressed(Button::DPAD_DOWN)) {
            player1.energy -= MOVEMENT_COST;
            channels[0].trigger_attack();
            //thrust(player1.position, player1.velocity);
        } else {
            channels[0].trigger_release();
        }
    } else {
        channels[0].trigger_release();
    }

    // Player loses invincibility if they shoot
    if(buttons.pressed & Button::A) {
        player1.invincible = false;
    }

    if (buttons.pressed & Button::A && time - player1.t_shot_fired > 50 && player1.energy >= LASER_COST) {
        player1.shot_fired = true;
        player1.t_shot_fired = time;

        // Offet the shot to just in front of the players nose
        Vec2 shot_offset(0.0f, -10.0f);
        shot_offset.rotate(-player1.rotation);
        player1.shot_origin = player1.position + shot_offset;

        // Create a line extrued out from the shot origin
        Vec2 beam(0, 0);
        beam -= Vec2(0, 300);
        beam.rotate(-player1.rotation);
        beam += player1.shot_origin;
        player1.shot_target = Vec2(beam.x, beam.y);

        channels[1].frequency = 2000;
        channels[1].trigger_attack();
        player1.energy -= LASER_COST;
    }

    // Decay the shot and explosion frequency
    if(channels[1].frequency  > 0) {
        channels[1].frequency *= 0.98f;
    }

    if(channels[2].frequency  > 0) {
        channels[2].frequency *= 0.98f;
    }

    movement.rotate(-player1.rotation);
    player1.velocity += movement;
    player1.velocity *= 0.99f;
    player1.position += player1.velocity;

    player1.rotational_velocity *= 0.95f;
    player1.rotation += player1.rotational_velocity;

    std::vector<polygon> new_polygons;

    bool player_inside_asteroid = false;

    for(auto &p: polygons) {
        bool do_split = false;
        uint32_t player_collissions = 0;
        Vec2 last_point = p.points.back();
        for(auto &point: p.points){
            Vec2 nope;
            // Count the number of time a line projected out to the right of the player's origin
            // collides with the line segment of this polygon
            if(!player1.invincible && !player_inside_asteroid && line_segment_intersection(&nope, last_point, point, player1.position, player1.position + Vec2(1000.0f, player1.position.y))){
                player_collissions++;
            }

            if(player1.shot_fired && !do_split){
                do_split = line_segment_intersection(&nope, last_point, point, player1.shot_origin, player1.shot_target);
            }

            last_point = point;
        }
        // If the projected player line collides an *odd* number of times then we know the player is inside a polygon
        if(player_collissions & 0b1) {
            player_inside_asteroid = true;
        }

        // If the player's shot intersects any line in this polygon we must slice it into twos
        if(do_split){
            polygon poly = split_polygon(&p, player1.shot_origin, player1.shot_target);
            if(poly.points.size()) {
                new_polygons.push_back(poly);
            }
        }
    }
    player1.shot_fired = false;

    if(player_inside_asteroid) {
        player1.energy -= DAMAGE_COST;
    }

    for(auto &polygon: new_polygons){
        polygons.push_back(polygon);
    }

    new_polygons.clear();
    polygons.erase(std::remove_if(polygons.begin(), polygons.end(), prune_polygons), polygons.end());

    for(auto &polygon: polygons) {
        polygon.rotational_velocity *= 0.9999f;
        polygon.velocity *= 0.9999f;
        rotate_polygon(polygon.points, polygon.rotational_velocity, polygon.origin);
        translate_polygon(polygon, polygon.velocity);

        Vec2 offset(0, 0);

        for(auto &p : polygon.points) {
            if(p.x > screen.bounds.w - 1) {
                offset.x = std::min(offset.x, screen.bounds.w - p.x);
            } else if (p.x < 0) {
                offset.x = std::max(offset.x, std::abs(p.x));
            }
            if(p.y > screen.bounds.h - 1) {
                offset.y = std::min(offset.y, screen.bounds.h - p.y);
            } else if (p.y < 0) {
                offset.y = std::max(offset.y, std::abs(p.y));
            }
        }

        if(offset.x){
            polygon.velocity.x *= -1;
            polygon.rotational_velocity += 0.0005f;
        }
        if(offset.y){
            polygon.velocity.y *= -1;
            polygon.rotational_velocity += 0.0005f;
        }

        translate_polygon(polygon, offset);
    }

    Vec2 offset(0, 0);

    std::vector<Vec2> player1_shape(player1.shape);
    translate_polygon(player1_shape, player1.position);
    rotate_polygon(player1_shape, player1.rotation, player1.position);

    for(auto &p: player1_shape) {
        if(p.x > screen.bounds.w - 1) {
            offset.x = std::min(offset.x, screen.bounds.w - p.x);
        } else if (p.x < 0) {
            offset.x = std::max(offset.x, std::abs(p.x));
        }
        if(p.y > screen.bounds.h - 1) {
            offset.y = std::min(offset.y, screen.bounds.h - p.y);
        } else if (p.y < 0) {
            offset.y = std::max(offset.y, std::abs(p.y));
        }
    }

    if(offset.x){
        player1.velocity.x *= -1;
    }
    if(offset.y){
        player1.velocity.y *= -1;
    }

    player1.position += offset;

    if(player1.energy < 0) {
        explode(player1.position);
        if(player1.lives > 0) {
            player1.lives--;
        }
        player1.reset_or_die();
    }

    if(player1.energy <= STARTING_ENERGY) {
        player1.energy += REGEN_ENERGY;
    }
}

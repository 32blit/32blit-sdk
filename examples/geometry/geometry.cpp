#include "geometry.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

#define COLLECT_THRESHOLD (unsigned short)150
#define DISCARD_THRESHOLD (unsigned short)20
#define STARTING_ENERGY (unsigned short)1000
#define LASER_COST (unsigned short)10
#define MOVEMENT_COST (float)0.01

#define ASTEROID_COUNT (unsigned short)8
#define ASTEROID_MIN_SIZE (unsigned short)4
#define ASTEROID_MAX_SIZE (unsigned short)15

#define POLYGON_COUNT (unsigned int)10


typedef struct player {
    vec2 velocity;
    vec2 position;
    float rotation = 0;
    float rotational_velocity = 0;
    std::vector<vec2> shape;
    float energy = 0;
    unsigned int score = 0;
    bool shot_fired = false;
    unsigned int t_shot_fired = 0;
    vec2 shot_origin;
    vec2 shot_target;
} player;

typedef struct polygon {
    float colour_offset;
    vec2 velocity;
    float rotational_velocity = 0;
    vec2 origin;
    std::vector<vec2> points;
    bool prune = false;
} polygon;

std::vector<polygon> polygons;

player player1;

rect bounds_of_polygon(std::vector<vec2> &points) {
    float xmin=1000000, xmax=-1000000, ymin=1000000, ymax=-1000000;

    for (auto &p : points) {
        if (p.x < xmin) xmin=p.x;
        if (p.x > xmax) xmax=p.x;
        if (p.y < ymin) ymin=p.y;
        if (p.y > ymax) ymax=p.y;
    }
    return rect(xmin, ymin, xmax-xmin, ymax-ymin);
}

vec2 middle_of_polygon(std::vector<vec2> &points) {
    rect bounds = bounds_of_polygon(points);
    return vec2(
        bounds.x + (bounds.w / 2),
        bounds.y + (bounds.h / 2)
    );
}

bool line_segment_intersection(vec2 *intersection, vec2 a, vec2 b, vec2 c, vec2 d) {
    vec2 r = b - a;
    vec2 s = d - c;

    float rxs = (r.x * s.y) - (r.y * s.x);
    float u = ((c.x - a.x) * r.y - (c.y - a.y) * r.x) / rxs;
    float t = ((c.x - a.x) * s.y - (c.y - a.y) * s.x) / rxs;

    if (0 <= u && u <= 1 && 0 <= t && t <= 1) {
        *intersection = a + (r * t);
        return true;
    }

    return false;
}

polygon split_polygon(polygon *poly, vec2 a, vec2 b) {
    std::vector<vec2> split_a;
    std::vector<vec2> split_b;

    vec2 last_point = poly->points.back();

    for (auto &p : poly->points) {
        float bside = ((p.x - a.x) * (b.y - a.y)) - ((p.y - a.y) * (b.x - a.x));
        float last_bside = ((last_point.x - a.x) * (b.y - a.y)) - ((last_point.y - a.y) * (b.x - a.x));

        if (bside != last_bside) {
            vec2 intersection;
            if (line_segment_intersection(&intersection, last_point, p, a, b)) {
                split_a.push_back(vec2(intersection.x, intersection.y));
                split_b.push_back(vec2(intersection.x, intersection.y));
            }
        }

        if (bside > 0) {
            split_a.push_back(vec2(p.x, p.y));
        }
        else {
            split_b.push_back(vec2(p.x, p.y));
        }

        last_point = p;
    }

    if (split_b.size() > 0) {
        poly->points = std::vector<vec2>(split_a);
        poly->origin = middle_of_polygon(poly->points);
        poly->rotational_velocity += 0.005;

        rect bounds = bounds_of_polygon(poly->points);
        poly->prune = (bounds.w * bounds.h) < 70;

        bounds = bounds_of_polygon(split_b);
        if((bounds.w * bounds.h) >= 70){
            polygon new_polygon;
            new_polygon.colour_offset = poly->colour_offset;
            new_polygon.velocity = poly->velocity * -1;
            new_polygon.points = std::vector<vec2>(split_b);
            new_polygon.origin = middle_of_polygon(new_polygon.points);
            new_polygon.rotational_velocity = -poly->rotational_velocity;
            return new_polygon;
        }
        else {
            // TODO: Add particle effect for destroyed polygon
        }
    }
    
    polygon new_polygon;
    return new_polygon;
}

void draw_polygon(std::vector<vec2> &points) {
    vec2 last_point = points[0];
    for (auto &p : points) {
        fb.line(last_point, p);
        last_point = p;
    }
    fb.line(last_point, points[0]);
}

std::vector<vec2> random_convex_polygon(vec2 origin, float radius) {
    unsigned int count = (rand() % 7) + 3;
    origin += vec2(radius, radius);
    std::vector<float> angles;
    for (auto a = 0; a < count; a++) {
        angles.push_back(float(rand() % 360) * M_PI / (float)180);
    }
    std::sort(angles.begin(), angles.end());
    std::vector<vec2> points;
    for (auto &angle : angles) {
        vec2 p = vec2(0, -radius);
        p.rotate(angle);
        points.push_back(p + origin);
    }
    return points;
}

void rotate_polygon(std::vector<vec2> &points, float angle, vec2 origin) {
    mat3 t = mat3::identity();
    t *= mat3::translation(origin);
    t *= mat3::rotation(angle);
    t *= mat3::translation(-origin);
    for (auto &p : points) {
        p *= t;
    }
}

void translate_polygon(polygon &polygon, vec2 translation) {
    mat3 t = mat3::identity();
    t *= mat3::translation(translation);
    for (auto &p : polygon.points) {
        p *= t;
    }
    polygon.origin *= t;
}

void translate_polygon(std::vector<vec2> &points, vec2 translation) {
    mat3 t = mat3::identity();
    t *= mat3::translation(translation);
    for (auto &p : points) {
        p *= t;
    }
}

void rotate_polygon(std::vector<vec2> &points, float angle) {
    vec2 origin = middle_of_polygon(points);
    rotate_polygon(points, angle, origin);
}

void init() {
    set_screen_mode(screen_mode::hires);
    for(int i = 0; i < POLYGON_COUNT; i++){
        polygon p;
        float x = rand() % fb.bounds.w;
        float y = rand() % fb.bounds.h;
        //float x = fb.bounds.w / 2;
        //float y = fb.bounds.h / 2;
        float r = (rand() % 20) + 10;
        float vx = ((rand() % 10) - 5) / 25.0;
        float vy = ((rand() % 10) - 5) / 25.0;
        p.colour_offset = (rand() % 100) / 100.0;
        p.points = random_convex_polygon(vec2(x, y), r);
        p.velocity = vec2(vx, vy);
        p.origin = middle_of_polygon(p.points);
        p.rotational_velocity = ((rand() % 20) - 10) / (float)360.0;
        polygons.push_back(p);
    }

    player1.position = vec2(fb.bounds.w / 2, fb.bounds.h / 2);
    player1.shape.push_back(vec2(0, -6));
    player1.shape.push_back(vec2(-6, 6));
    player1.shape.push_back(vec2(0, 2));
    player1.shape.push_back(vec2(6, 6));
}

void render(uint32_t time) {
    uint32_t ms_start = now();
    float h = time / (M_PI * 2) / 100.0;

    fb.pen(rgba(0, 0, 0));
    fb.clear();

    fb.pen(rgba(255, 255, 255));
    std::vector<vec2> player1_shape(player1.shape);
    rotate_polygon(player1_shape, player1.rotation);
    translate_polygon(player1_shape, player1.position);
    draw_polygon(player1_shape);

    for(auto &p: polygons){
        rgba c = hsv_to_rgba(h / (M_PI * 2) + p.colour_offset, 1.0, 1.0);
        fb.pen(c);
        draw_polygon(p.points);
        //fb.pixel(p.origin);
    }

    if(time - player1.t_shot_fired > 0 && time - player1.t_shot_fired < 500){
        int c = 255 - ((time - player1.t_shot_fired ) / 2);
        fb.pen(rgba(c, c, c));
        fb.line(player1.shot_origin, player1.shot_target);
    }

    uint32_t ms_end = now();

    // draw FPS meter
    fb.alpha = 255;
    fb.pen(rgba(255, 255, 255, 100));
    fb.rectangle(rect(1, fb.bounds.h - 10, 12, 9));
    fb.pen(rgba(255, 255, 255, 200));
    std::string fms = std::to_string(ms_end - ms_start);
    fb.text(fms, &minimal_font[0][0], rect(3, fb.bounds.h - 9, 10, 16));

    int block_size = 4;
    for (int i = 0; i < (ms_end - ms_start); i++) {
      fb.pen(rgba(i * 5, 255 - (i * 5), 0));
      fb.rectangle(rect(i * (block_size + 1) + 1 + 13, fb.bounds.h - block_size - 1, block_size, block_size));
    }
}

bool prune_polygons(polygon p){
    // TODO: Add particle effect for destroyed polygons
    return p.prune;
}

bool split = false;
void update(uint32_t time) {

    vec2 movement(0, 0);

    if (pressed(button::DPAD_LEFT))  { player1.rotational_velocity += M_PI / 720; }
    if (pressed(button::DPAD_RIGHT)) { player1.rotational_velocity -= M_PI / 720; }
    if (pressed(button::DPAD_UP))    { movement.y -= 0.03f; }
    if (pressed(button::DPAD_DOWN))  { movement.y += 0.03f; }

    player1.rotational_velocity += joystick.x * M_PI / 720;
    movement.y += joystick.y / 10.0f;

    if (pressed(button::A) && time - player1.t_shot_fired > 500) {
        player1.shot_fired = true;
        player1.t_shot_fired = time;
        player1.shot_origin = player1.position;
        vec2 beam(0, 0);
        beam -= vec2(0, 200);
        beam.rotate(-player1.rotation);
        beam += player1.shot_origin;
        player1.shot_target = vec2(beam.x, beam.y);
    }

    movement.rotate(-player1.rotation);
    player1.velocity += movement;
    player1.velocity *= 0.99f;
    player1.position += player1.velocity;

    player1.rotational_velocity *= 0.95f;
    player1.rotation += player1.rotational_velocity;

    std::vector<polygon> new_polygons;

    if(player1.shot_fired){
        for(auto &p: polygons) {

            bool do_split = false;
            vec2 last_point = p.points.back();
            for(auto &point: p.points){
                vec2 nope;
                do_split = line_segment_intersection(&nope, last_point, point, player1.shot_origin, player1.shot_target);
                if(do_split) break;
                last_point = point;
            }

            if(do_split){
                polygon poly = split_polygon(&p, player1.shot_origin, player1.shot_target);
                if(poly.points.size()) {
                    new_polygons.push_back(poly);
                }
            }
        }
        player1.shot_fired = false;
    }

    for(auto &polygon: new_polygons){
        polygons.push_back(polygon);
    }

    polygons.erase(std::remove_if(polygons.begin(), polygons.end(), prune_polygons), polygons.end());

    for(auto &polygon: polygons) {
        polygon.rotational_velocity *= 0.999;
        rotate_polygon(polygon.points, polygon.rotational_velocity, polygon.origin);
        translate_polygon(polygon, polygon.velocity);

        vec2 offset(0, 0);

        for(auto &p : polygon.points) {
            if(p.x > fb.bounds.w - 1) {
                offset.x = std::min(offset.x, fb.bounds.w - p.x);
            } else if (p.x < 0) {
                offset.x = std::max(offset.x, abs(p.x));
            }
            if(p.y > fb.bounds.h - 1) {
                offset.y = std::min(offset.y, fb.bounds.h - p.y);
            } else if (p.y < 0) {
                offset.y = std::max(offset.y, abs(p.y));
            }
        }

        if(offset.x){
            polygon.velocity.x *= -1;
            polygon.rotational_velocity += 0.0005;
        }
        if(offset.y){
            polygon.velocity.y *= -1;
            polygon.rotational_velocity += 0.0005;
        }

        translate_polygon(polygon, offset);
    }

    vec2 offset(0, 0);

    std::vector<vec2> player1_shape(player1.shape);
    rotate_polygon(player1_shape, player1.rotation);
    translate_polygon(player1_shape, player1.position);

    for(auto &p: player1_shape) {
        if(p.x > fb.bounds.w - 1) {
            offset.x = std::min(offset.x, fb.bounds.w - p.x);
        } else if (p.x < 0) {
            offset.x = std::max(offset.x, abs(p.x));
        }
        if(p.y > fb.bounds.h - 1) {
            offset.y = std::min(offset.y, fb.bounds.h - p.y);
        } else if (p.y < 0) {
            offset.y = std::max(offset.y, abs(p.y));
        }
    }

    if(offset.x){
        player1.velocity.x *= -1;
    }
    if(offset.y){
        player1.velocity.y *= -1;
    }

    player1.position += offset;
}

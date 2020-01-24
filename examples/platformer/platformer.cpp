
#include <map>
#include <string>
#include <vector>
#include <cstdlib>

#include "platformer.hpp"

using namespace blit;

size screen_size(160, 120);

/* define storage for the framebuffer, spritesheet, and mask */
rgba    __ss[128 * 128] __SECTION__(".ss");
uint8_t __m[320 * 240] __SECTION__(".m");  

/* create surfaces */
surface m((uint8_t *)__m, pixel_format::M, screen_size);

const int max_light_radius = 60;
uint8_t __mshad[(max_light_radius * 2 + 1) * (max_light_radius * 2 + 1)] __SECTION__(".m");
surface mshad((uint8_t *)__mshad, pixel_format::M, size(max_light_radius * 2 + 1, max_light_radius * 2 + 1));

point world_to_screen(const vec2 &p);
point world_to_screen(const point &p);
point screen_to_world(const point &p);
void highlight_tile(point p, rgba c);
point tile(const point &p);
point player_origin();
void draw_layer(MapLayer &layer);
void draw_flags();
void render_light(point pt, float radius, bool shadows);
void blur(uint8_t passes);
void bloom(uint8_t passes);
void load_assets();


/* create map */
enum TileFlags { SOLID = 1, WATER = 2, LADDER = 4 };
Map map(rect(0, 0, 48, 24));

uint8_t player_animation[5] = { 208, 209, 210, 211, 212 };
float player_animation_frame = 0;

float clamp(float v, float min, float max) {
  return v > min ? (v < max ? v : max) : min;
}

struct Player {
  vec2 vel;
  vec2 pos;
  bool flip = false;

  enum {
    STILL,
    WALKING,
    JUMPING
  };
  std::map<uint8_t, std::vector<uint8_t>> animations;
  float animation_frame = 0.0f;

  Player() {
    animations[STILL] = { 208, 208, 208, 208, 208, 208, 209, 208, 208, 208, 208, 208, 208, 208 };
    animations[WALKING] = { 208, 209, 210, 211, 212 };
    animations[JUMPING] = { 217 };

    vel = vec2(0, 0);
    pos = vec2(100, 32);
  }

  rect aabb() {
    return rect(pos.x - 4, pos.y - 12, 7, 11);
  }

  uint8_t animation_sprite_index(uint8_t animation) {
    uint8_t animation_length = animations[animation].size();
    return animations[animation][uint32_t(animation_frame) % animation_length];
  }

  point current_tile() {
    return point(pos.x / 8, pos.y / 8);
  }

  bool tile_under_solid() {
    point p = current_tile();
    return map.has_flag(p, TileFlags::SOLID);
  }

  bool tile_under_ladder() {
    point p = current_tile();
    return map.tile_index(p) == 7;
  }

  bool on_ground() {
    if (vel.y < 0) return false;

    point p = current_tile();
    return map.has_flag(point(p.x, p.y + 1), TileFlags::SOLID) && ((int32_t(pos.y) % 8) == 7);
  }

  bool in_water() {
    return false;
    point p = current_tile();
    return map.has_flag(point(p.x, p.y + 1), TileFlags::WATER);
  }

  /*
    return a clipped camera point that doesn't allow the viewport to
    leave the world bounds
  */
  point camera() {      
    static rect b(screen_size.w / 2, screen_size.h / 2, map.bounds.w * 8 - fb.bounds.w, map.bounds.h * 8 - fb.bounds.h);
    return b.clamp(point(floor(pos.x), floor(pos.y)));
  }

  rect viewport() {
    point c = camera();
    rect vprect(
      c.x - fb.bounds.w / 2,
      c.y - fb.bounds.h / 2,
      fb.bounds.w,
      fb.bounds.h
    );
  }

  void update() {
    static float max_speed_x = 2.0f;
    static float ground_acceleration_x = 0.5f;
    static float air_acceleration_x = 0.2f;
    static float duration = 0.01f;
    static vec2 gravity(0, 9.8f * 5.0f);   // normal gravity is boring!

    if (pressed(button::DPAD_LEFT)) {
      vel.x = vel.x - (on_ground() ? ground_acceleration_x : air_acceleration_x);
    }

    if (pressed(button::DPAD_RIGHT)) {
      vel.x = vel.x + (on_ground() ? ground_acceleration_x : air_acceleration_x);
    }

    vel.x = clamp(vel.x, -max_speed_x, max_speed_x);
    //player.vel = player.vel + (gravity * duration);

    vec2 future_player_pos = pos + vel;
    rect future_bb = rect(future_player_pos.x - 4, future_player_pos.y - 12, 7, 11);

    bool collision = false;
    map.tiles_in_rect(future_bb, [&collision](point tile_pt) -> void {
      if (map.has_flag(tile_pt, TileFlags::SOLID)) {
        collision = true;
      }
    });

    if (!collision) {
      pos = future_player_pos;
    }
    else {
      vel.x = 0.0f;
      vel.y = 0.0f;
    }
  }

  void render() {
    uint8_t animation = Player::STILL;

    if (abs(vel.x) > 1) {
      animation = Player::WALKING;
    }
    if (!on_ground()) {
      animation = Player::JUMPING;
    }

    uint8_t si = animation_sprite_index(animation);

    point sp = world_to_screen(point(pos.x - 4, pos.y - 7));
    fb.sprite(si, sp, flip);
    sp.y -= 8;
    fb.sprite(si - 16, sp, flip);


    rect bb = aabb();
    fb.pen(rgba(0, 255, 0));
    fb.line(world_to_screen(bb.tl()), world_to_screen(bb.tr()));
    fb.line(world_to_screen(bb.bl()), world_to_screen(bb.br()));

    map.tiles_in_rect(bb, [&bb](point tile_pt) -> void {
      point sp = world_to_screen(tile_pt * 8);
      rect rb(sp.x, sp.y, 8, 8);

      fb.pen(rgba(0, 255, 0, 150));
      if (map.has_flag(tile_pt, TileFlags::SOLID)) {
        fb.pen(rgba(255, 0, 0, 150));
      }

      fb.rectangle(rb);
    });

    //map.tiles_in_rect(bb)
  }
} player;


struct bat {
  vec2 pos;
  vec2 vel = vec2(-2, 0);
  uint8_t current_frame = 0;
  std::array<uint8_t, 6> frames = {{ 96, 97, 98, 99, 98, 97 }};

  void update() {
    current_frame++;
    current_frame %= frames.size();

    vec2 test_pos = pos + (vec2::normalize(vel) * 8.0f);

    if (map.has_flag(tile(point(test_pos)), TileFlags::SOLID)) {
      vel.x *= -1;
    }

    pos += vel;
  }
};

bat bat1;

struct slime {
  vec2 pos;
  vec2 vel = vec2(1, 0);
  uint8_t current_frame = 0;
  std::array<uint8_t, 4> frames = {{ 112, 113, 114, 113 }};

  void update() {
    current_frame++;
    current_frame %= frames.size();

    vec2 test_pos = pos + (vec2::normalize(vel) * 8.0f);

    if (map.has_flag(tile(point(test_pos)), TileFlags::SOLID)) {
      vel.x *= -1;
    }

    pos += vel;
  }
};

slime slime1;

void animation_timer_callback(timer &timer) {
  bat1.update();
  slime1.update();
}

timer t;






/* setup */
void init() {
  load_assets();

  bat1.pos = vec2(200, 22);
  slime1.pos = vec2(50, 112);

  t.init(animation_timer_callback, 50, -1);
  t.start();

  screen_size.w = 160;
  screen_size.h = 120;
  //engine::set_screen_mode(screen_mode::hires);
}


void render(uint32_t time) {
  static int32_t x = 0; x++;
      
  uint32_t ms_start = now();

  fb.mask = nullptr;
  fb.alpha = 255;
  fb.pen(rgba(0, 0, 0));
  fb.clear();

  // mask out for lighting    
  mshad.alpha = 255;
  mshad.pen(rgba(0));
  mshad.clear();

  m.alpha = 255;
  m.pen(rgba(64));
  m.clear();

  // render lights
  
  for (uint8_t y = 0; y < 24; y++) {
    for (uint8_t x = 0; x < 48; x++) {
      uint32_t ti = map.layers["effects"].tile_at(point(x, y));
      point lp = point(x * 8 + 4, y * 8 + 3);
      if (ti == 37 || ti == 38) {
        render_light(lp, 15.0f, false);
      }
    }
  }

  render_light(point(player.pos.x, player.pos.y - 7), 60.0f, true);

  // light up the "outside" this should be done with map flags
  rect r; m.pen(rgba(255));
  r = rect(world_to_screen(point(0, 0)), world_to_screen(point(112, 40))); m.rectangle(r);
  r = rect(world_to_screen(point(0, 40)), world_to_screen(point(80, 48))); m.rectangle(r);
  r = rect(world_to_screen(point(0, 48)), world_to_screen(point(72, 56))); m.rectangle(r);
  r = rect(world_to_screen(point(0, 56)), world_to_screen(point(48, 64))); m.rectangle(r);
  r = rect(world_to_screen(point(0, 64)), world_to_screen(point(32, 72))); m.rectangle(r);

  bloom(3);
  blur(1);


  fb.alpha = 255;
  fb.pen(rgba(39, 39, 54));
  fb.clear();



  // draw world
  // layers: background, environment, effects, characters, objects
  draw_layer(map.layers["background"]);
  draw_layer(map.layers["environment"]);
  draw_layer(map.layers["effects"]);
  draw_layer(map.layers["objects"]);


  // draw player
  player.render();


  // bat
  point sp = world_to_screen(point(bat1.pos.x - 4, bat1.pos.y));
  fb.sprite(bat1.frames[bat1.current_frame], sp, bat1.vel.x < 0 ? false : true);

  // slime
  sp = world_to_screen(point(slime1.pos.x - 4, slime1.pos.y));
  fb.sprite(slime1.frames[slime1.current_frame], sp, slime1.vel.x < 0 ? false : true);


  // overlay water
  fb.pen(rgba(56, 136, 205, 125));
  for (uint8_t y = 0; y < 24; y++) {
    for (uint8_t x = 0; x < 48; x++) {
      point pt = world_to_screen(point(x *   8, y * 8));
      uint32_t ti = map.tile_index(point(x, y));

      if (map.has_flag(point(x, y), TileFlags::WATER)) {
        fb.rectangle(rect(pt.x, pt.y, 8, 8));
      }
    }
  }

  // invert the lighting mask
  m.custom_modify(m.clip, [](uint8_t *p, int16_t c) -> void {
    while (c--) {
      *p = 255 - *p;
      p++;
    }
  });
  
  // blend over lighting
  fb.mask = &m;
  fb.pen(rgba(39 / 2, 39 / 2, 54 / 2));
  fb.clear();

  static int tick = 0;
  tick++;
  fb.mask = nullptr;
  fb.alpha = 255;
  fb.sprite(139, point(2, 2));
  fb.sprite(139, point(12, 2));
  fb.sprite(139, point(22, 2));

  
  // draw FPS meter
  uint32_t ms_end = now();
  fb.mask = nullptr;
  fb.pen(rgba(255, 0, 0));
  for (int i = 0; i < (ms_end - ms_start); i++) {
    fb.pen(rgba(i * 5, 255 - (i * 5), 0));
    fb.rectangle(rect(i * 3 + 1, 117, 2, 2));
  }
  

  // highlight current player tile
  // point pt2 = player.current_tile();
  // highlight_tile(pt2, rgba(0, 255, 0, 100));

  // draw map flags
  // draw_flags();
}


/*
  update() is called every 10ms, all effects should be
  scaled to that duration

  player velocity is in tiles per second, so if the players
  'x' velocity is 1 then they move sideways by one tile per
  second

  one tile is considered to be 1 metre
*/
void update(uint32_t time) {
  float duration = 0.01f;     // == 10ms as a float


  static float jump_velocity = 15.0f;
  static vec2 gravity(0, 9.8f * 5.0f);   // normal gravity is boring!

  static bool jumping = false;


  player.update();

  /*
  // player is on the ground and not moving left or right, friction!
  if (player.in_water()) {
    if ((pressed(button::A) | pressed(button::DPAD_UP))) {
      player.vel.y = 0;
    }

    if ((pressed(button::A) | pressed(button::DPAD_LEFT))) {
      player.vel.x = 0;
    }

    if ((pressed(button::A) | pressed(button::DPAD_RIGHT))) {
      player.vel.x = 0;
    }

  }
  else {
    if (pressed(button::DPAD_LEFT)) {
      player.vel.x = player.vel.x - (player.on_ground() ? ground_acceleration_x : air_acceleration_x);
      player.vel.x = std::max(-max_speed_x, player.vel.x);
      player.flip = true;
    }

    if (pressed(button::DPAD_RIGHT)) {
      player.vel.x = player.vel.x + (player.on_ground() ? ground_acceleration_x : air_acceleration_x);
      player.vel.x = std::min(max_speed_x, player.vel.x);
      player.flip = false;
    }

    if (player.on_ground()) {
      jumping = false;

      if (!pressed(button::DPAD_RIGHT) && !pressed(button::DPAD_LEFT)) {
        player.vel.x = player.vel.x * 0.90;
      }

      if ((pressed(button::A) | pressed(button::DPAD_UP)) && !jumping) {
        player.vel.y = -jump_velocity;
        jumping = true;
      }
    }
  }


  //if player_on_ground() and math.abs(last_noise_x - player.pos.x) > 5 then
  //  step_noise()
  //  last_noise_x = player.pos.x
  //end

  point old_tile = tile(player_origin());


  float water = player.in_water() ? 0.2f : 1.0f;
  player.vel = player.vel + (gravity * duration * water);

  point new_tile = tile(player_origin());


  // collision detection and correction

  // falling    
  if (player.tile_under_solid() && old_tile.y < new_tile.y) {
//      player.pos.y = old_tile.y * 8 + 7;
    player.vel.y = 0;
  }

  // moving left/right
  if (!map.has_flag(old_tile, TileFlags::SOLID) && map.has_flag(new_tile, TileFlags::SOLID) && ((old_tile.x > new_tile.x) || (old_tile.x < new_tile.x))) {
    player.pos.x = old_tile.x * 8 + 4;
    player.vel.x = 0;
  }


  player.animation_frame += 0.1;
  if (player.on_ground()) {

    player.pos.y = floor(player.pos.y);

    if (player.vel.y > 0) {
      player.vel.y = 0;
    }
  }
  else {
    player_animation_frame = 2;
  }
  /*
  if (tick_seed % 3 == 0) {
    for (uint8_t y = 0; y < 16; y++) {
      for (uint8_t x = 0; x < 32; x++) {
        uint8_t ti = tile_index(point(x, y));
        if (ti >= 5 && ti <= 7) {
          set_tile_index(point(x, y), 5 + rand() % 3);
        }
      }
    }
  }*/

}




float orient2d(vec2 p1, vec2 p2, vec2 p3) {
  return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}

std::vector<std::pair<vec2, vec2>> get_occluders(point pt, float radius) {
  std::vector<std::pair<vec2, vec2>> occluders;

  rect light_bounds(pt, pt);
  light_bounds.inflate(max_light_radius);

  map.tiles_in_rect(light_bounds, [&occluders, &pt](point tile_pt) -> void {
    if (map.has_flag(tile_pt, TileFlags::SOLID)) {
      rect rb(tile_pt.x * 8, tile_pt.y * 8, 8, 8);
      rb.x -= pt.x;
      rb.y -= pt.y;

      vec2 o(0, 0);
      vec2 fpt(pt.x, pt.y);
      /*vec2 tl(rb.x - 0.5f, rb.y + 0.5f);// = rb.tl();
      vec2 tr(rb.x + rb.w - 0.5f, rb.y + 0.5f);// = rb.tr();
      vec2 bl(rb.x - 0.5f, rb.y + rb.h + 0.5f);// = rb.bl();
      vec2 br(rb.x + rb.w - 0.5f, rb.y + rb.h + 0.5f);// = rb.br();
      */
      vec2 tl = vec2(rb.tl().x - 1, rb.tl().y - 1);
      vec2 tr = vec2(rb.tr().x - 1, rb.tr().y - 1);
      vec2 bl = vec2(rb.bl().x - 1, rb.bl().y - 1);
      vec2 br = vec2(rb.br().x - 1, rb.br().y - 1);

      if (!map.has_flag(point(tile_pt.x, tile_pt.y + 1), TileFlags::SOLID) && orient2d(bl, br, o) > 0) {
        occluders.push_back(std::make_pair(bl, br));
      }

      if (!map.has_flag(point(tile_pt.x - 1, tile_pt.y), TileFlags::SOLID) && orient2d(tl, bl, o) > 0) {
        occluders.push_back(std::make_pair(tl, bl));
      }

      if (!map.has_flag(point(tile_pt.x, tile_pt.y - 1), TileFlags::SOLID) && orient2d(tr, tl, o) > 0) {
        occluders.push_back(std::make_pair(tr, tl));
      }

      if (!map.has_flag(point(tile_pt.x + 1, tile_pt.y), TileFlags::SOLID) && orient2d(br, tr, o) > 0) {
        occluders.push_back(std::make_pair(br, tr));
      }
    }
  });

  return occluders;
}

void render_light(point pt, float radius, bool shadows = false) {
  point lpt(max_light_radius, max_light_radius);

  mshad.alpha = 255;
  mshad.pen(rgba(0));
  mshad.clear();

  // draw the light aura
  mshad.alpha = (rand() % 10) + 40;
  int steps = 20;
  for (int j = steps; j > 0; j--) {
    mshad.pen(rgba(255));
    mshad.circle(lpt, (j * radius / steps));
  }

  if (shadows)
  {
    // cut out the shadows
    mshad.alpha = 255;
    mshad.pen(rgba(0));

    float rs = radius * radius;
    std::vector<std::pair<vec2, vec2>> occluders = get_occluders(pt, radius);
    for (auto occluder : occluders) {
      vec2 p1 = occluder.first;
      vec2 p2 = occluder.second;
      vec2 fpt(pt.x, pt.y);

      vec2 rv1 = p1;
      vec2 rv2 = p2;

      if ((abs(rv1.x) * abs(rv1.y)) < rs && (abs(rv2.x) * abs(rv2.y)) < rs) {
        // (max_light_radius * 2) = cludge to ensure shadows are projected far enough
        // actually we should project shadows to the bounds of the light bounding box
        // there is no need to "guess" but that requires working out the intersection
        // with the edge of the bounding box and optionally inserting points at the corners
        // if required. a task for another day....
        float c1 = (max_light_radius * 2) / float(std::max(abs(rv1.x), abs(rv1.y)));
        float c2 = (max_light_radius * 2) / float(std::max(abs(rv2.x), abs(rv2.y)));

        vec2 p3 = rv1 * c1;
        vec2 p4 = rv2 * c2;

        point wp1 = p1 + lpt;
        point wp2 = p2 + lpt;
        point wp3 = p3 + lpt;
        point wp4 = p4 + lpt;

        std::vector<point> poly = {
          wp1, wp3, wp4, wp2
        };

        mshad.triangle(wp1, wp2, wp3);
        mshad.triangle(wp2, wp4, wp3);
        //mshad.polygon(poly);
      }
    }
  }

  point light_corner = world_to_screen(pt - point(max_light_radius, max_light_radius));
  m.custom_blend(&mshad, mshad.clip, light_corner, [](uint8_t *psrc, uint8_t *pdest, int16_t c) -> void {
    while (c--) {
      *pdest = *pdest ^ ((*pdest ^ *psrc) & -(*pdest < *psrc)); // integer `max` without branching

      pdest++;
      psrc++;
    }
  });
}

void blur(uint8_t passes) {
  uint8_t last;

  for (uint8_t pass = 0; pass < passes; pass++) {
    uint8_t *p = (uint8_t *)m.data;
    for (uint16_t y = 0; y < m.bounds.h; y++) {
      last = *p;
      p++;

      for (uint16_t x = 1; x < m.bounds.w - 1; x++) {
        *p = (*(p + 1) + last + *p + *p) >> 2;
        last = *p;
        p++;
      }

      p++;
    }
  }

  // vertical      
  for (uint8_t pass = 0; pass < passes; pass++) {
    for (uint16_t x = 0; x < m.bounds.w; x++) {
      uint8_t *p = (uint8_t *)m.data + x;

      last = *p;
      p += m.bounds.w;

      for (uint16_t y = 1; y < m.bounds.h - 1; y++) {
        *p = (*(p + m.bounds.w) + last + *p + *p) >> 2;
        last = *p;
        p += m.bounds.w;
      }
    }
  }
}


  void bloom(uint8_t passes) {
  for (uint8_t pass = 0; pass < passes; pass++) {
    uint8_t *p = (uint8_t *)m.data + m.bounds.w;
    for (uint16_t y = 1; y < m.bounds.h - 1; y++) {
      p++;

      for (uint16_t x = 1; x < m.bounds.w - 1; x++) {
        uint8_t v1 = *p;
        uint8_t v2 = *(p + 1);
        uint8_t v3 = *(p + m.bounds.w);
        *p++ = v1 > v2 ? (v1 > v3 ? v1 : v3) : (v2 > v3 ? v2 : v3);
      }

      p++;
    }
    
    p = (uint8_t *)m.data + (m.bounds.h * m.bounds.w) - 1 - m.bounds.w;
    for (uint16_t y = 1; y < m.bounds.h - 1; y++) {
      p--;

      for (uint16_t x = 1; x < m.bounds.w - 1; x++) {
        uint8_t v1 = *p;
        uint8_t v2 = *(p - 1);
        uint8_t v3 = *(p - m.bounds.w);
        *p-- = v1 > v2 ? (v1 > v3 ? v1 : v3) : (v2 > v3 ? v2 : v3);
      }

      p--;
    }
  }
}

void load_assets() {
  std::vector<uint8_t> layer_background = { 17,17,17,17,17,17,17,17,17,17,17,17,17,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,47,17,17,17,17,17,17,17,17,17,17,17,17,17,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,47,17,17,17,17,17,17,17,17,17,17,17,17,17,17,0,0,0,0,0,0,0,0,0,0,0,0,0,41,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,0,0,0,47,1,2,3,4,1,2,3,1,2,3,4,5,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,47,0,0,0,0,0,0,0,51,0,0,0,13,14,0,41,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,68,47,0,0,0,30,0,0,0,0,15,0,0,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,31,84,47,0,0,0,0,0,0,0,15,0,0,0,0,0,0,0,0,0,0,0,0,31,67,47,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,0,0,0,0,0,0,0,0,0,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,83,47,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,223,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,0,0,0,0,0,0,41,0,0,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,0,0,0,0,0,0,0,0,15,0,78,0,0,0,0,0,0,0,0,15,0,0,78,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,15,15,15,15,15,41,15,15,15,41,15,41,15,15,15,41,0,0,0,15,15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,41,15,41,15,15,15,15,41,15,15,15,15,15,60,15,15,0,0,0,15,41,41,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,15,60,15,15,15,15,15,41,15,15,41,15,15,15,41,15,41,41,15,15,15,15,41,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,15,15,15,41,15,13,15,15,41,15,15,41,15,41,15,15,15,15,41,15,15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,15,41,15,15,41,15,15,15,41,41,15,15,15,15,15,30,15,15,15,41,15,60,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,15,15,15,15,15,41,15,15,15,60,15,15,41,15,15,15,15,41,15,41,15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };;
  map.add_layer("background", layer_background);

  std::vector<uint8_t> layer_environment = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,72,0,74,28,29,60,0,0,0,0,0,0,60,28,29,0,0,15,0,0,0,0,0,0,0,0,0,0,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,72,0,74,44,45,0,0,0,0,0,0,0,0,44,45,0,0,0,0,15,0,0,0,0,15,0,0,0,44,45,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,72,0,74,0,0,0,0,0,0,0,0,0,0,60,89,89,89,89,71,0,0,0,0,0,0,0,0,0,74,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,88,89,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,28,29,0,0,0,0,0,15,0,74,0,0,0,0,0,0,0,0,0,0,0,0,0,0,190,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,44,45,0,0,15,0,0,0,0,74,0,0,0,0,0,0,0,0,0,0,0,0,0,0,121,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,71,0,0,0,0,0,0,74,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,44,45,58,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,88,89,89,89,89,89,71,74,0,0,0,0,0,0,0,0,0,0,0,60,57,57,87,0,0,74,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,0,0,0,0,0,60,74,0,0,0,56,57,0,0,0,0,56,57,87,0,0,0,0,0,86,57,57,57,57,28,29,57,60,57,57,57,50,55,55,55,55,48,57,57,57,60,0,0,0,0,0,0,0,28,29,55,55,55,64,0,58,7,57,60,72,0,0,0,0,75,94,94,94,94,94,94,76,44,45,0,0,0,0,0,66,16,16,48,49,127,0,0,28,29,60,0,0,0,0,0,0,44,45,16,16,16,64,0,74,7,0,0,72,0,0,0,0,79,0,0,0,0,0,0,77,0,75,94,94,94,94,76,126,49,49,127,0,0,0,0,44,45,66,55,55,55,55,55,55,60,16,16,16,16,64,0,74,7,0,0,93,94,94,94,94,95,0,0,0,0,0,0,93,94,95,0,0,0,0,47,0,0,0,0,0,0,0,0,0,0,66,16,16,16,16,16,16,16,16,16,16,16,64,0,74,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,47,0,0,0,0,0,0,0,0,0,0,126,49,49,49,50,16,16,16,16,16,16,48,127,0,31,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,47,0,0,0,0,0,0,0,0,0,0,0,0,0,0,66,16,16,16,16,16,48,127,0,0,31,7,0,0,0,0,0,0,0,0,40,61,62,62,63,0,0,0,0,0,0,0,0,47,0,0,0,0,0,0,0,0,0,0,0,0,0,0,126,49,49,49,49,49,127,0,0,0,91,62,62,62,62,62,62,62,62,62,62,92,0,0,91,62,62,63,0,61,62,62,62,92,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,79,0,77,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,79,59,77,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  map.add_layer("environment", layer_environment);
  map.layers["environment"].add_flags({ 8, 59, 31, 47, 28, 29, 44, 45, 60, 48, 49, 50, 64, 66, 80, 81, 82, 56, 57, 58, 72, 74, 88, 89, 90, 61, 62, 63, 77, 79, 93, 94, 95 }, TileFlags::SOLID);
  map.layers["environment"].add_flags({ 7 }, TileFlags::LADDER);
  map.layers["environment"].add_flags({ 16, 55, 223 }, TileFlags::WATER);

  std::vector<uint8_t> layer_effects = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,53,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,0,0,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,52,52,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  map.add_layer("effects", layer_effects);

  std::vector<uint8_t> layer_characters = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,96,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,0,0,0,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,208,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,96,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,144,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,166,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,122,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,182,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  map.add_layer("characters", layer_characters);

  std::vector<uint8_t> layer_objects = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,68,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,84,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,25,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,0,39,0,0,0,0,0,0,0,0,68,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,39,0,0,0,0,0,0,0,0,0,0,85,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,84,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  map.add_layer("objects", layer_objects);

  fb.sprites = spritesheet::load(packed_data);
}


point world_to_screen(const vec2 &p) {
  return point(
    p.x - player.camera().x + m.bounds.w / 2,
    p.y - player.camera().y + m.bounds.h / 2
  );
}

point world_to_screen(const point &p) {
  return point(
    p.x - player.camera().x + m.bounds.w / 2,
    p.y - player.camera().y + m.bounds.h / 2
  );
}

point screen_to_world(const point &p) {
  return point(
    p.x + player.camera().x - m.bounds.w / 2,
    p.y + player.camera().y - m.bounds.h / 2
  );
}


void highlight_tile(point p, rgba c) {
  fb.pen(c);
  p.x *= 8;
  p.y *= 8;
  p = world_to_screen(p);
  fb.rectangle(rect(p.x, p.y, 8, 8));
}

point player_origin() {
  return point(player.pos.x, player.pos.y);
}

point tile(const point &p) {
  return point(p.x / 8, p.y / 8);
}


void draw_layer(MapLayer &layer) {
  point tl = screen_to_world(point(0, 0));
  point br = screen_to_world(point(fb.bounds.w, fb.bounds.h));

  point tlt = tile(tl);
  point brt = tile(br);

  for (uint8_t y = tlt.y; y <= brt.y; y++) {
    for (uint8_t x = tlt.x; x <= brt.x; x++) {
      point pt = world_to_screen(point(x * 8, y * 8));
      uint32_t ti = layer.map->tile_index(point(x, y));
      if (ti != -1) {
        uint8_t si = layer.tiles[ti];
        if (si != 0) {
          fb.sprite(si, pt);
        }
      }
    }
  }
}

rgba flag_colours[] = {
  rgba(255, 0, 0, 100),
  rgba(0, 255, 0, 100),
  rgba(0, 0, 255, 100)
};

void draw_flags() {
  for (uint8_t y = 0; y < 24; y++) {
    for (uint8_t x = 0; x < 48; x++) {
      point pt = world_to_screen(point(x * 8, y * 8));
      uint32_t ti = map.tile_index(point(x, y));
      uint8_t f = map.get_flags(point(x, y));

      for (uint8_t i = 0; i < 3; i++) {
        if (f & (1 << i)) {
          fb.pen(flag_colours[i]);
          fb.rectangle(rect(pt.x, pt.y, 8, 8));
        }
      }
    }
  }
}
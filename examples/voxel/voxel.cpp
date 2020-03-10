/* 
  32blit Voxel Terrain demo

  by Jonathan Williamson (lowfatcode)
*/

#include <cstring>

#include "32blit.hpp"
#include "map.hpp"

using namespace blit;


Pen sky_colour = Pen(127, 182, 212, 255);
Pen colour_map_palette[256];

uint16_t *tiles[32][32];
std::vector<uint16_t *> free_tiles;

constexpr uint16_t tile_cache_size = 120;
uint16_t tile_cache[32 * 32 * tile_cache_size];

Vec3 position(64, 64, 100);
float angle = 0.0f;
float lean = 0.0f;
float pitch = 0.0f;

uint8_t visited[32][32];

uint32_t terrain_file;
uint8_t terrain_index = 1;



void load_map() {
  //close_file(terrain_file);
  //terrain_file = open_file(std::to_string(terrain_index) + ".map");

  // load palette data 
  //if(read_file(terrain_file, 0, 768, (char *)buffer) == 768) {
  for(uint16_t i = 0; i < 256; i++) {
    colour_map_palette[i] = Pen(demo_map[i * 3 + 0], demo_map[i * 3 + 1], demo_map[i * 3 + 2]);
  }
  //}

  for(uint8_t y = 0; y < 32; y++) {
    for(uint8_t x = 0; x < 32; x++) {
      tiles[x][y] = nullptr;
    }
  }

  free_tiles.clear();
  for(uint8_t i = 0; i < tile_cache_size; i++) {
    free_tiles.push_back(&tile_cache[32 * 32 * i]);
  }
}

void init() {
  set_screen_mode(ScreenMode::lores);
  load_map();
}

void load_tile(int16_t x, int16_t y) {
  uint32_t tile_size = 32 * 32 * 2;
  uint32_t offset = 768 + ((x + y * 32) * tile_size);

  if(free_tiles.size() > 0) {
    tiles[x][y] = free_tiles.back();
    free_tiles.pop_back();
    
    memcpy((char *)tiles[x][y], demo_map + offset, tile_size);

    //read_file(terrain_file, offset, tile_size, (char *)tiles[x][y]);
  }
}

uint16_t get_sample(int16_t x, int16_t y) {
  // work out the tile coordinates for this sample  
  uint8_t tx = (x >> 5) & 0x1f;
  uint8_t ty = (y >> 5) & 0x1f;

  visited[tx][ty] = 1;

  if(tiles[tx][ty] == nullptr) {
    return 0;
  }

  // work out the coordinates (within the tile) for the sample
  uint16_t sx = x & 0x1f;
  uint16_t sy = y & 0x1f;

  return tiles[tx][ty][sx + sy * 32];
}

float deg2rad(float d) {
  return d * pi / 180.0f;
}

void draw_world(Vec3 position, float angle, float lean, float horizon, float near, float far) {
  static int16_t height_buffer[160];  
  
  memset(visited, 0, 32 * 32);

  // reset the height buffer to the bottom of the screen
  for(auto i = 0; i < screen.bounds.w; i++) {
    height_buffer[i] = screen.bounds.h;
  }
 
  // convert angle into radians
  angle = deg2rad(angle);
  float sina = sinf(angle);
  float cosa = cosf(angle);

  // starting z value
  float z = near;
  
  while(z < far) { 

    // calculate the left and right Points for the current sample span
    Vec2 frustrum_left  = Vec2(-cosa * z - sina * z,  sina * z - cosa * z);
    Vec2 frustrum_right = Vec2( cosa * z - sina * z, -sina * z - cosa * z);

    // calculate the step size along the span for each screen column drawn
    Vec2 sample_step((frustrum_right.x - frustrum_left.x) / screen.bounds.w, (frustrum_right.y - frustrum_left.y) / screen.bounds.w);
    float lean_step = lean * 2.0f / screen.bounds.w;

    // pre-multiply the fog alpha for this z-distance
    uint8_t fog_blend = (z / far / 1.5f) * 255; // value for "amount of fog" from 0 to 255
    Pen fog((sky_colour.r * fog_blend) >> 8, (sky_colour.g * fog_blend) >> 8, (sky_colour.b * fog_blend) >> 8);
    fog_blend = 255 - fog_blend;

    // Point being sampled, starts on the left of the view frustrum and is stepped forward each
    // screen column drawn until it reaches the right of the view frustrum
    Vec2 sample_point = frustrum_left + Vec2(position.x, position.y);
    float sample_lean = -lean;

    float invz = 1.0f / z * 100.0f;

    // for each column on the screen...
    for(uint16_t i = 0; i < screen.bounds.w; i++) {

      uint16_t sample = get_sample(sample_point.x, sample_point.y);
      uint8_t colour_index = sample >> 8;
     
      // convert the height map sample into a y coordinate on screen      
      int16_t height = (position.z - (sample & 0xff)) * invz + float(horizon) + sample_lean;

      // if the height is smaller (further up the screen) than our current height buffer
      // value then we need to draw a new vertical strip
      if(height < height_buffer[i]) {  
        // fetch the colour for this strip from the colour map
        Pen colour = colour_map_palette[colour_index];

        // blend terrain colour with pre-multiplied fog colour
        colour.r = ((colour.r * fog_blend) >> 8) + fog.r;
        colour.g = ((colour.g * fog_blend) >> 8) + fog.g;
        colour.b = ((colour.b * fog_blend) >> 8) + fog.b;
        
        // draw the vertical strip
        screen.pen = colour;
        screen.v_span(Point(i, height), height_buffer[i] - height); 

        // update the height buffer to save that we've draw this far
        // up the screen
        height_buffer[i] = height;
      }
                
      // move to the next sampling coordinate
      sample_point += sample_step;
      sample_lean += lean_step;
    }

    // move forward (into the distance) in increasingly large steps
    z *= 1.025f;
  }
}



void render(uint32_t time_ms) {

  // draw the sky
  screen.pen = sky_colour;
  screen.clear();

  uint32_t ms_start = now();
  draw_world(
    position, // player position
    angle, // player direction
    lean,
    20.0f + pitch, // horizon position
    3.0f,   // near distance
    300.0f  // far distance
  ); 
  uint32_t ms_end = now();  

  for(uint8_t y = 0; y < 32; y++) {
    for(uint8_t x = 0; x < 32; x++) {

      if(tiles[x][y] != nullptr) {
        screen.pen = Pen(255, 0, 0, 100);
      }else{
        screen.pen = Pen(0, 0, 0, 100);
      }
      screen.pixel(Point(x, y));

      if(visited[x][y]) {
        screen.pen = Pen(255, 255, 0, 100);
        screen.pixel(Point(x, y));
      }
    }
  }

  // work out the tile coordinates for the player position
  uint8_t tx = (int32_t(position.x) >> 5) & 0x1f;
  uint8_t ty = (int32_t(position.y) >> 5) & 0x1f;
  screen.pen = Pen(0, 255, 0);
  screen.pixel(Point(tx, ty));

  // draw FPS meter & watermark
  screen.watermark();
  screen.mask = nullptr;
  screen.pen = Pen(255, 255, 255);
  
  screen.text(std::to_string(ms_end - ms_start), minimal_font, Point(1, 230));
  screen.pen = Pen(255, 0, 0);
  for (int i = 0; i < uint16_t(ms_end - ms_start); i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(Rect(i * 3 + 1, 237, 2, 2));
  }  
}

void update(uint32_t time_ms) {
  static uint16_t tick = 0;
  static uint32_t last_buttons = 0;
  
  tick++;

  // update angle of player based on joystick input
  float old_angle = angle;
  angle -= joystick.x * 2.0f;  

  lean = (angle - old_angle) * 10.0f;

  if(buttons & DPAD_UP) {
    position.z += 1.0f;
  }

  if(buttons & DPAD_DOWN) {
    position.z -= 1.0f;
  }

  // move player location if joystick y axis is forward/backwards
  position.x += sinf(deg2rad(angle)) * joystick.y;
  position.y += cosf(deg2rad(angle)) * joystick.y;
  pitch = joystick.y * 20.0f;

  position.x = position.x < 0.0f ? 1023.0f : position.x;
  position.y = position.y < 0.0f ? 1023.0f : position.y;

  position.x = position.x > 1023.0f ? 0.0f : position.x;
  position.y = position.y > 1023.0f ? 0.0f : position.y;

   
  // remove any unused tiles from the cache
  for(uint8_t y = 0; y < 32; y++) {
    for(uint8_t x = 0; x < 32; x++) {
      if(tiles[x][y] != nullptr && !visited[x][y]) {
        free_tiles.push_back(tiles[x][y]);
        tiles[x][y] = nullptr;
      }
    }
  }

  // load the next needed tile into the cache
  for(uint8_t y = 0; y < 32; y++) {
    for(uint8_t x = 0; x < 32; x++) {
      if(tiles[x][y] == nullptr && visited[x][y]) {
        load_tile(x, y);
        //return; // quit out early if we load a new tile
      }
    }
  }

  if(!(buttons & A) && (last_buttons & A)) {
    terrain_index++;
    if(terrain_index > 4) {
      terrain_index = 1;
    }
    load_map();
  }

  last_buttons = buttons;
}
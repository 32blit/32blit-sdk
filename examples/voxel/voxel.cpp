#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "voxel.hpp"

using namespace blit;

rgba sky_colour = rgba(127, 182, 212, 255);
vec3 position(64, 64, 200);
float angle = 0.0f;

void init() {
  // sad trombone
}

void draw_world(vec3 position, float angle, int horizon, float near, float far) {
  static int16_t height_buffer[160];
  
  // reset the height buffer to the bottom of the screen
  for(auto i = 0; i < fb.bounds.w; i++) {
    height_buffer[i] = fb.bounds.h;
  }
 
  float sina = sin(angle);
  float cosa = cos(angle);

  // starting z value
  float z = near;
  
  while(z < far) { 

    // calculate the left and right points for the current sample span
    vec2 frustrum_left  = vec2(-cosa * z - sina * z,  sina * z - cosa * z);
    vec2 frustrum_right = vec2( cosa * z - sina * z, -sina * z - cosa * z);

    // calculate the step size along the span for each screen column drawn
    vec2 step((frustrum_right.x - frustrum_left.x) / fb.bounds.w, (frustrum_right.y - frustrum_left.y) / fb.bounds.w);
    
    // pre-multiply the fog alpha for this z-distance
    uint8_t fog_blend = (z / far / 1.2f) * 255; // value for "amount of fog" from 0 to 255
    rgba fog((sky_colour.r * fog_blend) >> 8, (sky_colour.g * fog_blend) >> 8, (sky_colour.b * fog_blend) >> 8);
    fog_blend = 255 - fog_blend;

    // point being sampled, starts on the left of the view frustrum and is stepped forward each
    // screen column drawn until it reaches the right of the view frustrum
    vec2 sample_point = frustrum_left + vec2(position.x, position.y);

    float invz = 1.0f / z * 10.0f;

    // for each column on the screen...
    for(uint8_t i = 0; i < fb.bounds.w; i++) {

      // determine offset of sample from heightmap and colour map
      uint16_t sample_offset = (int8_t(sample_point.x) & 0x7f) + (int8_t(sample_point.y) & 0x7f) * 128;
      
      // convert the height map sample into a y coordinate on screen
      int height = (position.z - height_map[sample_offset]) * invz + float(horizon);

      // if the height is smaller (further up the screen) than our current height buffer
      // value then we need to draw a new vertical strip
      if(height < height_buffer[i]) {  
        // fetch the colour for this strip from the colour map
        rgba colour = colour_map_palette[colour_map[sample_offset]];

        // blend terrain colour with pre-multiplied fog colour
        colour.r = ((colour.r * fog_blend) >> 8) + fog.r;
        colour.g = ((colour.g * fog_blend) >> 8) + fog.g;
        colour.b = ((colour.b * fog_blend) >> 8) + fog.b;
        
        // draw the vertical strip
        fb.pen(colour);
        fb.v_span(point(i, height), height_buffer[i] - height); 

        // update the height buffer to save that we've draw this far
        // up the screen
        height_buffer[i] = height;
      }
                
      // move to the next sampling coordinate
      sample_point += step;
    }

    // move forward (into the distance) in increasingly large steps
    z *= 1.025f;
  }
}

void render(uint32_t time_ms) {

  // draw the sky
  fb.pen(sky_colour);
  fb.clear();  

  uint32_t ms_start = now();
  draw_world(
    position, // player position
    angle, // player direction
    0, // horizon position
    3.0f,   // near distance
    500.0f  // far distance
  ); 
  uint32_t ms_end = now();

  

  // draw FPS meter & watermark
  fb.watermark();
  fb.mask = nullptr;
  fb.pen(rgba(255, 255, 255));
  fb.text(std::to_string(ms_end - ms_start), &minimal_font[0][0], point(1, 110));
  fb.pen(rgba(255, 0, 0));
  for (int i = 0; i < uint16_t(ms_end - ms_start); i++) {
    fb.pen(rgba(i * 5, 255 - (i * 5), 0));
    fb.rectangle(rect(i * 3 + 1, 117, 2, 2));
  }
  
}

void update(uint32_t time_ms) {
  static uint16_t tick = 0;
  tick++;

  // update angle of player based on joystick input
  angle -= joystick.x / 25.0f;  

  // clip players z position to ensure they are above the ground
  uint16_t sample_offset = (int8_t(position.x) & 0x7f) + (int8_t(position.y) & 0x7f) * 128;
  if(position.z < (height_map[sample_offset] + 50)) {
    position.z = height_map[sample_offset] + 50;
  }

  // move player location if joystick y axis is forward/backwards
  position.x += sin(angle) * joystick.y;
  position.y += cos(angle) * joystick.y;

}
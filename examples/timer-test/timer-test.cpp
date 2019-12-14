#include <string>
#include <string.h>
#include <memory>
#include <cstdlib>

#include "timer-test.hpp"

/*
TODO: This example is really dry, how can we make it awesome?
Without making it so complicated that it fails to elucidate its point.
*/

using namespace blit;

const uint16_t screen_width = 320;
const uint16_t screen_height = 240;

blit::timer timer_count;
uint32_t count;

void timer_count_update(blit::timer &t){
    count++;
    // Instead of using loops we're going to stop the timer in our callback
    // In this case it will count to ten and then stop.
    // But you could depend upon any condition to stop the timer.
    if(count == 10) {
        t.stop();
    }
}

void init() {
  blit::set_screen_mode(blit::screen_mode::hires);

  // Timers must be initialized
  // In this case we want our timer to call the `timer_count_update` function
  // very 1000ms, or 1 second. We also want it to loop indefinitely.
  // We can pass -1 to loop indefinitely, or just nothing at all since -1
  // is the default.
  timer_count.init(timer_count_update, 1000, -1);

  // Next we probably want to start our timer!
  timer_count.start();
}

int tick_count = 0;
void render(uint32_t time_ms) {
  char text_buffer[60];
  fb.pen(rgba(20, 30, 40));
  fb.clear();
  
  // Fancy title bar, nothing to see here.
  fb.pen(rgba(255, 255, 255));
  fb.rectangle(rect(0, 0, 320, 14));
  fb.pen(rgba(0, 0, 0));
  fb.text("Timer Test", &minimal_font[0][0], point(5, 4));

  // Since our timer callback is updating our `count` variable
  // we can just display it on the screen and watch it tick up!
  fb.pen(rgba(255, 255, 255));
  sprintf(text_buffer, "Count: %d", count);
  fb.text(text_buffer, &minimal_font[0][0], point(120, 100));

  // `is_running()` is a handy shorthand for checking the timer state
  if(timer_count.is_running()) {
    fb.text("Timer running...", &minimal_font[0][0], point(120, 110));
  } else {
    fb.text("Timer stopped!", &minimal_font[0][0], point(120, 110));
    fb.text("Press A to restart.", &minimal_font[0][0], point(120, 120));
  }
}

void update(uint32_t time_ms) {
  // `is_stopped()` works too!
  if (blit::buttons & blit::button::A && timer_count.is_stopped()) {
      count = 0;
      timer_count.start();
  }
}
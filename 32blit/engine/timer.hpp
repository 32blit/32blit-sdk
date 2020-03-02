#pragma once

#include <cstdint>
#include <vector>

namespace blit {

  struct Timer {
    typedef void (*TimerCallback)(Timer &timer);

    // uint32_t callback;                      // reference to Lua callback function (can be obtained via `ref = _G['function_name']`)
    
    TimerCallback callback = nullptr;
   
    uint32_t duration = 0;                  // how many milliseconds between callbacks
    uint32_t started = 0;                   // system time when timer started in milliseconds
    int16_t loops = -1;                     // number of times to repeat timer (-1 == forever)
    enum state {                            // state of the timer 
      STOPPED, 
      RUNNING, 
      PAUSED,
      FINISHED
    };
    uint8_t state = STOPPED;

    void init(TimerCallback callback, uint32_t duration, int32_t loops);
    void start();
    void stop();

    bool is_running()   { return this->state == RUNNING; }
    bool is_paused()    { return this->state == PAUSED; }
    bool is_stopped()   { return this->state == STOPPED; }
    bool is_finished()  { return this->state == FINISHED; }

    Timer();
  };

  extern std::vector<Timer *> timers;

  struct timer_event_t {

  };

  extern void update_timers(uint32_t time);

  //extern std::vector<timer *> timers;  

}
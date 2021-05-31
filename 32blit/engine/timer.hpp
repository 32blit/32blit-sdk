#pragma once

#include <cstdint>
#include <vector>

namespace blit {

  struct Timer {
    using TimerCallback = std::function<void (Timer &timer)>;

    TimerCallback callback = nullptr;
    void *user_data = nullptr;

    uint32_t duration = 0;                  // how many milliseconds between callbacks
    uint32_t started = 0;                   // system time when timer started in milliseconds
    int32_t loops = -1, orig_loops = -1;    // number of times to repeat timer (-1 == forever)
    enum state {                            // state of the timer
      UNINITIALISED,
      STOPPED,
      RUNNING,
      PAUSED,
      FINISHED
    };
    uint8_t state = UNINITIALISED;

    void init(TimerCallback callback, uint32_t duration, int32_t loops = -1);
    void start();
    void stop();

    bool is_running()   { return this->state == RUNNING; }
    bool is_paused()    { return this->state == PAUSED; }
    bool is_stopped()   { return this->state == STOPPED; }
    bool is_finished()  { return this->state == FINISHED; }

    Timer();
    Timer(TimerCallback callback, uint32_t duration, int32_t loops = -1);
    ~Timer();
  };

  extern void update_timers(uint32_t time);
}

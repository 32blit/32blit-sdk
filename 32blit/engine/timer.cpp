/*! \file timer.cpp
*/
#include "engine.hpp"
#include "timer.hpp"

namespace blit {
  std::vector<Timer *> timers;

  Timer::Timer() = default;

  Timer::Timer(TimerCallback callback, uint32_t duration, int32_t loops) {
    init(callback, duration, loops);
  }

  Timer::~Timer() {
    for(auto it = timers.begin(); it != timers.end(); ++it) {
      if(*it == this) {
        timers.erase(it);
        break;
      }
    }
  }

  /**
   * Initialize the timer.
   *
   * @param callback Callback function to trigger when timer has elapsed.
   * @param duration Duration of the timer in milliseconds.
   * @param loops Number of times the timer should repeat, -1 = forever.
   */
  void Timer::init(TimerCallback callback, uint32_t duration, int32_t loops) {
    this->callback = callback;
    this->duration = duration;
    this->loops = loops;
  }

  /**
   * Start the timer.
   */
  void Timer::start() {
    if(state == UNINITIALISED)
      timers.push_back(this);

    if(state == PAUSED)
      started = blit::now() - (paused - started); // Modify start time based on when timer was paused.
    else {
      started = blit::now();
      loop_count = 0;
    }

    this->state = RUNNING;
  }

  /**
   * Pause the timer.
   */
  void Timer::pause() {
    if (state != RUNNING) return;

    paused = blit::now();
    state = PAUSED;
  }

  /**
   * Stop the running timer.
   */
  void Timer::stop() {
    if(state == UNINITIALISED) return;

    this->state = STOPPED;
  }

  /**
   * Update all running timers, triggering any that have elapsed.
   *
   * @param time Time in milliseconds.
   */
  void update_timers(uint32_t time) {
    for (auto t: timers) {
      if (t->state == Timer::RUNNING){
        if (time > (t->started + t->duration)) { // timer triggered
          if(t->loops == -1){
            t->started = time; // reset the start time correcting for any error
          }
          else
          {
            t->loop_count++;
            t->started = time;
            if (t->loop_count == t->loops){
              t->state = Timer::FINISHED;
            }
          }
          t->callback(*t);
        }
      }
    }
  }
}

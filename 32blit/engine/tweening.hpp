#pragma once

#include <cstdint>
#include <string>

namespace blit {
  const uint32_t LINEAR = 1UL << 0;

  struct Tween {
    typedef float (*TweenFunction)(uint32_t t, float b, float c, uint32_t d);
    TweenFunction function = nullptr;

    float from = 0.0f;
    float to = 1.0f;
    float value = 0.0f;

    uint32_t duration = 0;
    int32_t loops = -1;
    int32_t loop_count = 0;
    uint32_t started = 0;

    enum state {
      STOPPED, 
      RUNNING, 
      PAUSED,
      FINISHED
    };
    uint8_t state = STOPPED;

    void init(TweenFunction function, float start, float end, uint32_t duration, int32_t loops);
    void start();
    void stop();

    bool is_running()   { return this->state == RUNNING; }
    bool is_paused()    { return this->state == PAUSED; }
    bool is_stopped()   { return this->state == STOPPED; }
    bool is_finished()  { return this->state == FINISHED; }
  };

  extern std::vector<Tween *> tweens;

  void update_tweens(uint32_t time);

  float tween_sine(uint32_t t, float b, float c, uint32_t d);
  float tween_linear(uint32_t t, float b, float c, uint32_t d);
  float tween_ease_in_quad(uint32_t t, float b, float c, uint32_t d);
  float tween_ease_out_quad(uint32_t t, float b, float c, uint32_t d);
  float tween_ease_in_out_quad(uint32_t t, float b, float c, uint32_t d);
}
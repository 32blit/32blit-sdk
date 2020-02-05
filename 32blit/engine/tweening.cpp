#include <vector>
#include <algorithm>
#include <cmath>

#include "engine.hpp"
#include "tweening.hpp"

#undef M_PI
#define M_PI           3.14159265358979323846f  /* pi */

namespace blit {
  std::vector<Tween *> tweens;

  /**
   * Initialize the tween.
   *
   * @param function Callback function to provide tween value.
   * @param value to tween from
   * @param value to tween to
   * @param duration Duration of the tween in milliseconds.
   * @param loops Number of times the tween should repeat, -1 = forever.
   */
  void Tween::init(TweenFunction function, float from, float to, uint32_t duration, int32_t loops = -1) {
    this->loops = loops;
    this->function = function;
    this->from = from;
    this->to = to;
    this->duration = duration;
    this->loop_count = 0;
    tweens.push_back(this);
  }

  /**
   * Start the tween.
   */
  void Tween::start() {
    this->started = blit::now();
    this->loop_count = 0;
    this->state = RUNNING;
  }

  /**
   * Stop the running tween.
   */
  void Tween::stop() {
    this->state = STOPPED;
  }

  float tween_sine(uint32_t t, float b, float c, uint32_t d) {
    return b + (sin((float(t) / float(d) * M_PI * 2.0f) + (M_PI / 2.0f)) + 1.0f) / 2.0f * (c - b);
  }

  float tween_linear(uint32_t t, float b, float c, uint32_t d) {
    return c * t / d + b;
  }

  float tween_ease_in_quad(uint32_t t, float b, float c, uint32_t d) {
    float ft = float(t) / float(d);
    return -c * ft * (ft - 2) + b;
  }

  float tween_ease_out_quad(uint32_t t, float b, float c, uint32_t d) {
    float ft = float(t) / float(d);
    return c * ft * ft + b;
  }

  float tween_ease_in_out_quad(uint32_t t, float b, float c, uint32_t d) {
    float ft = (float)t / d / 2.0f;
    if (ft < 1) return c / 2 * ft * ft + b;
    ft--;
    return -c / 2 * (ft*(ft - 2) - 1) + b;
  }

  /**
   * Update tweens.
   *
   * @param time in milliseconds.
   */
  void update_tweens(uint32_t time) {
    for (auto tween : tweens) {
      if (tween->state == Tween::RUNNING){
        uint32_t elapsed = blit::now() - tween->started;
        tween->value = tween->function(elapsed, tween->from, tween->to, tween->duration);

        if (elapsed >= tween->duration) {
          if(tween->loops == -1){
            tween->started = blit::now();
          }
          else
          {
            tween->loop_count++;
            tween->started = blit::now();
            if (tween->loop_count == tween->loops){
              tween->state = Tween::FINISHED;
            }
          }
        }
      }
    }
  }
}
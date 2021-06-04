#include <vector>
#include <algorithm>
#include <cmath>

#include "engine.hpp"
#include "tweening.hpp"

#include "../math/constants.hpp"

namespace blit {
  std::vector<Tween *> tweens;

  Tween::Tween() = default;

  Tween::Tween(TweenFunction function, float start, float end, uint32_t duration, int32_t loops) {
    init(function, start, end, duration, loops);
  }

  Tween::~Tween() {
    for(auto it = tweens.begin(); it != tweens.end(); ++it) {
      if(*it == this) {
        tweens.erase(it);
        break;
      }
    }
  }

  /**
   * Initialize the tween.
   *
   * @param function Callback function to provide tween value.
   * @param value to tween from
   * @param value to tween to
   * @param duration Duration of the tween in milliseconds.
   * @param loops Number of times the tween should repeat, -1 = forever.
   */
  void Tween::init(TweenFunction function, float from, float to, uint32_t duration, int32_t loops) {
    this->loops = loops;
    this->function = function;
    this->from = from;
    this->to = to;
    this->duration = duration;
    this->loop_count = 0;
  }

  /**
   * Start the tween.
   */
  void Tween::start() {
    if(state == UNINITIALISED)
      tweens.push_back(this);

    if(state == PAUSED) {
      started = blit::now() - (paused - started); // Modify start time based on when tween was paused.
    } else {
      this->started = blit::now();
      this->loop_count = 0;
      this->value = this->from;
    }

    this->state = RUNNING;
  }

  /**
   * Pause the tween.
   */
  void Tween::pause() {
    if (state != RUNNING) return;

    paused = blit::now();
    state = PAUSED;
  }

  /**
   * Stop the running tween.
   */
  void Tween::stop() {
    if(state == UNINITIALISED) return;

    this->state = STOPPED;
  }

  float tween_sine(uint32_t t, float b, float c, uint32_t d) {
    return b + (sinf((float(t) / float(d) * pi * 2.0f) + (pi / 2.0f)) + 1.0f) / 2.0f * (c - b);
  }

  float tween_linear(uint32_t t, float b, float c, uint32_t d) {
    return (c - b) * t / d + b;
  }

  float tween_ease_in_quad(uint32_t t, float b, float c, uint32_t d) {
    float ft = float(t) / float(d);
    return -(c - b) * ft * (ft - 2) + b;
  }

  float tween_ease_out_quad(uint32_t t, float b, float c, uint32_t d) {
    float ft = float(t) / float(d);
    return (c - b) * ft * ft + b;
  }

  float tween_ease_in_out_quad(uint32_t t, float b, float c, uint32_t d) {
    float ft = (float)t / d * 2.0f;
    if (ft < 1) return (c - b) / 2 * ft * ft + b;
    ft--;
    return -(c - b) / 2 * (ft*(ft - 2) - 1) + b;
  }

  /**
   * Update tweens.
   *
   * @param time in milliseconds.
   */
  void update_tweens(uint32_t time) {
    for (auto tween : tweens) {
      if (tween->state == Tween::RUNNING){
        uint32_t elapsed = time - tween->started;
        tween->value = tween->function(elapsed, tween->from, tween->to, tween->duration);

        if (elapsed >= tween->duration) {
          if(tween->loops == -1){
            tween->started = time;
          }
          else
          {
            tween->loop_count++;
            tween->started = time;
            if (tween->loop_count == tween->loops){
              tween->state = Tween::FINISHED;
            }
          }
        }
      }
    }
  }
}

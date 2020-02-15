#pragma once

#include "32blit.hpp"

namespace blit {

  class Camera {
  public:
    Vec3 position;                             // position
    Vec3 direction;                             // direction
    Vec3 up = Vec3(0.0f, 1.0f, 0.0f);  // up Vec3
    float fov = 60.0f;

  
    Camera(Vec3 p, Vec3 d, Vec3 u, float fov);
    Mat4 rotation_matrix();
    Mat4 translation_matrix();
    Mat4 ortho_projection_matrix(float width, float height);    
    Mat4 perspective_projection_matrix(Rect viewport, float near, float far);
  };

}
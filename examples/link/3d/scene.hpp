#pragma once

#include "32blit.hpp"

namespace blit {

  class AABB {
    Vec3 v1;
    Vec3 v2;
  };

  class SceneNode {
  public:
    Vec3 position;
    Vec3 direction;
    Vec3 up;
    AABB aabb;
  };

  class Material {
    std::string name;
    float alpha = 1.0f;
    Pen color;
    Surface* texture;
  };

  class Light : SceneNode {

  };

  class Object : SceneNode {

  };

  class Scene {
  public:
    std::vector<Light> lights;
    std::vector<Object> children;
    // lights
    // children
  };


  class Mesh {

  };

}
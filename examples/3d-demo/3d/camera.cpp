#include "camera.hpp"


namespace blit {
    
  Camera::Camera(Vec3 p, Vec3 d, Vec3 u, float fov = 90.0f) :position(p), direction(d), up(u), fov(fov) {
    d.normalize();
    u.normalize();
  }

  Mat4 Camera::translation_matrix() {
    return Mat4::translation(Vec3(-position.x, -position.y, -position.z));
  }

  Mat4 Camera::rotation_matrix() {
    Vec3 rn(direction);
    rn.normalize();

    Vec3 ru = up.cross(direction);
    ru.normalize();

    Vec3 rv = rn.cross(ru);

    Mat4 r = Mat4::identity();

    r.v00 = ru.x; r.v01 = ru.y; r.v02 = ru.z;
    r.v10 = rv.x; r.v11 = rv.y; r.v12 = rv.z;
    r.v20 = rn.x; r.v21 = rn.y; r.v22 = rn.z;
    
    return r;
  }

  Mat4 Camera::ortho_projection_matrix(float width, float height) {
    Mat4 r = Mat4::ortho(-width / 2.0f, width / 2.0f, height / 2.0f, -height / 2.0f, (width + height) / 4.0f, -(width + height) / 4.0f);

    return r;
  }

  Mat4 Camera::perspective_projection_matrix(Rect viewport, float near, float far) {
    Mat4 m;
    
    float scale = 1.0f / tanf(fov * 0.5f * pi / 180.0f);
    m.v00 = scale; // x coordinate scale
    m.v11 = scale; // y coordinate scale
    m.v22 = -far / (far - near);        // remap z 0 .. 1
    m.v32 = -far * near / (far - near); // remap z 0 .. 1
    //m.v22 = -(far + near) / (far - near); // remap z 0 .. 1
    //m.v32 = -(2.0f * far * near) / (far - near); // remap z 0 .. 1
    m.v23 = -1.0f;
    m.v33 = 0;

    return m;
  }

}
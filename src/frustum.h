#pragma once
#include "common.h"
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Plane {
  float a, b, c, d;

  void normalize_plane() {
    float mag = std::sqrt(a * a + b * b + c * c);
    a /= mag;
    b /= mag;
    c /= mag;
    d /= mag;
  }

  float distance_to_point(const glm::vec3& point) const {
    return a * point.x + b * point.y + c * point.z + d;
  }
};

class Frustum {
  enum FrustumSide {
    TOP = 0,
    BOTTOM,
    LEFT,
    RIGHT,
    NEAR,
    FAR,
  };

  Plane planes[6];

public:
  Frustum();
  void create_frustum_from_camera(const glm::mat4& comboMatrix);
  bool test_point(const glm::vec3& point);
};

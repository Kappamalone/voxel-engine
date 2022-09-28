#pragma once
#include "common.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Plane {
  glm::vec3 normal;
  glm::vec3 point;

  [[nodiscard]] float signed_distance(glm::vec3 r) const {
    PRINT("TEST: {} {}\n", glm::length(normal), glm::length(point));
    return glm::dot(normal, r - point);
  }

  void create_from_three_points(const glm::vec3 p0, const glm::vec3 p1,
                                const glm::vec3 p2) {
    const glm::vec3 line0 = glm::normalize(p1 - p0);
    const glm::vec3 line1 = glm::normalize(p2 - p1);
    normal = glm::cross(line1, line0);
    PRINT("LENGTH: {}\n", glm::length(normal));
    point = p0;
  }
};

enum class FrustumSide {
  BOTTOM = 0,
  TOP,
  NEAR,
  FAR,
  LEFT,
  RIGHT,
};

class Frustum {
  Plane planes[6];

  // n = near, t = top, l = left
  glm::vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
  float nearD, farD, ratio, angle, tang;
  float nw, nh, fw, fh;

public:
  Frustum(float fovy, float aspect_ratio, float znear, float zfar);
  void create_frustum_from_camera(const glm::vec3& p, const glm::vec3& l,
                                  const glm::vec3& u);

  bool test_point(const glm::vec3& point);
};

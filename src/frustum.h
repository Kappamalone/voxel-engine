#pragma once
#include "chunk.h" // for bounding box
#include "common.h"
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// bless
// https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
struct Plane {
  float a, b, c, d;

  void normalize_plane() {
    float mag = std::sqrt(a * a + b * b + c * c);
    a /= mag;
    b /= mag;
    c /= mag;
    d /= mag;
  }

  [[nodiscard]] float distance_to_point(const glm::vec3& point) const {
    return a * point.x + b * point.y + c * point.z + d;
  }

  // tests all 8 bb vertices against the plane
  [[nodiscard]] bool bounding_box_test(const BoundingBox& bb) const {
    // clang-format off
    // TODO: http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
    glm::vec3 vertices[8] = {
      glm::vec3(bb.min.x, bb.min.y, bb.min.z),
      glm::vec3(bb.min.x, bb.min.y, bb.max.z),
      glm::vec3(bb.max.x, bb.min.y, bb.max.z),
      glm::vec3(bb.max.x, bb.min.y, bb.min.z),

      glm::vec3(bb.min.x, bb.max.y, bb.min.z),
      glm::vec3(bb.min.x, bb.max.y, bb.max.z),
      glm::vec3(bb.max.x, bb.max.y, bb.max.z),
      glm::vec3(bb.max.x, bb.max.y, bb.min.z),
    };

    // clang-format on
    int intersection_count = 0;
    for (auto& vertice : vertices) {
      if (distance_to_point(vertice) >= 0) {
        intersection_count++;
      }
    }
    return intersection_count > 0;
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
  bool test_bounding_box(const BoundingBox& bounding_box);
};

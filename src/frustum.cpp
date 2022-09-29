#include "frustum.h"

Frustum::Frustum() {
}

// must be called every frame before performing frustum culling
void Frustum::create_frustum_from_camera(const glm::mat4& comboMatrix) {
  // Left clipping plane
  planes[LEFT].a = comboMatrix[0][3] + comboMatrix[0][0];
  planes[LEFT].b = comboMatrix[1][3] + comboMatrix[1][0];
  planes[LEFT].c = comboMatrix[2][3] + comboMatrix[2][0];
  planes[LEFT].d = comboMatrix[3][3] + comboMatrix[3][0];

  // Right clipping plane
  planes[RIGHT].a = comboMatrix[0][3] - comboMatrix[0][0];
  planes[RIGHT].b = comboMatrix[1][3] - comboMatrix[1][0];
  planes[RIGHT].c = comboMatrix[2][3] - comboMatrix[2][0];
  planes[RIGHT].d = comboMatrix[3][3] - comboMatrix[3][0];

  // Top clipping plane
  planes[TOP].a = comboMatrix[0][3] - comboMatrix[0][1];
  planes[TOP].b = comboMatrix[1][3] - comboMatrix[1][1];
  planes[TOP].c = comboMatrix[2][3] - comboMatrix[2][1];
  planes[TOP].d = comboMatrix[3][3] - comboMatrix[3][1];

  // Bottom clipping plane
  planes[BOTTOM].a = comboMatrix[0][3] + comboMatrix[0][1];
  planes[BOTTOM].b = comboMatrix[1][3] + comboMatrix[1][1];
  planes[BOTTOM].c = comboMatrix[2][3] + comboMatrix[2][1];
  planes[BOTTOM].d = comboMatrix[3][3] + comboMatrix[3][1];

  // Near clipping plane
  planes[NEAR].a = comboMatrix[0][3] + comboMatrix[0][2];
  planes[NEAR].b = comboMatrix[1][3] + comboMatrix[1][2];
  planes[NEAR].c = comboMatrix[2][3] + comboMatrix[2][2];
  planes[NEAR].d = comboMatrix[3][3] + comboMatrix[3][2];

  // Far clipping plane
  planes[FAR].a = comboMatrix[0][3] - comboMatrix[0][2];
  planes[FAR].b = comboMatrix[1][3] - comboMatrix[1][2];
  planes[FAR].c = comboMatrix[2][3] - comboMatrix[2][2];
  planes[FAR].d = comboMatrix[3][3] - comboMatrix[3][2];

  for (auto& plane : planes) {
    plane.normalize_plane();
  }
}

// true = inside frustum
bool Frustum::test_point(const glm::vec3& point) {
  for (auto& plane : planes) {
    if (plane.distance_to_point(point) < 0) {
      return false;
    }
  }
  return true;
}

bool Frustum::test_bounding_box(const BoundingBox& bounding_box) {
  for (auto& plane : planes) {
    if (!plane.bounding_box_test(bounding_box)) {
      return false;
    }
  }
  return true;
}

/*
// clang-format off
auto test = glm::mat4(0, 0, 0, 0,
                      1, 0, 0, 0,
                      0, 0, 0, 0,
                      0, 0, 0, 0);
// clang-format on
PANIC("{}\n", test[1][0]);
// therefore mat[row][column]
*/

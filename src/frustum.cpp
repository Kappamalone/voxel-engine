#include "frustum.h"

Frustum::Frustum() {
}

// must be called every frame before performing frustum culling
void Frustum::create_frustum_from_camera(const glm::mat4& comboMatrix) {
  // Left clipping plane
  /*
  planes[0].a = comboMatrix._41 + comboMatrix._11;
  planes[0].b = comboMatrix._42 + comboMatrix._12;
  planes[0].c = comboMatrix._43 + comboMatrix._13;
  planes[0].d = comboMatrix._44 + comboMatrix._14;
  */
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
  /*
  planes[2].a = comboMatrix._41 - comboMatrix._21;
  planes[2].b = comboMatrix._42 - comboMatrix._22;
  planes[2].c = comboMatrix._43 - comboMatrix._23;
  planes[2].d = comboMatrix._44 - comboMatrix._24;
  */
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
  /*
  planes[4].a = comboMatrix._41 + comboMatrix._31;
  planes[4].b = comboMatrix._42 + comboMatrix._32;
  planes[4].c = comboMatrix._43 + comboMatrix._33;
  planes[4].d = comboMatrix._44 + comboMatrix._34;
  */
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

  for (auto& plane : planes) {
    if (plane.distance_to_point(point) < 0) {
      return false;
    }
  }
  return true;
}

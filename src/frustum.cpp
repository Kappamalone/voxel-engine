#include "frustum.h"

Frustum::Frustum(float fovy, float aspect_ratio, float znear, float zfar) {
  // store the information
  this->ratio = aspect_ratio;
  this->angle = fovy;
  this->nearD = znear;
  this->farD = zfar;

  // compute width and height of the near and far plane sections
  tang = (float)tan(glm::radians(fovy) * angle * 0.5);
  nh = nearD * tang;
  nw = nh * ratio;
  fh = farD * tang;
  fw = fh * ratio;
}

// must be called every frame before performing frustum culling
void Frustum::create_frustum_from_camera(const glm::vec3& p, const glm::vec3& l,
                                         const glm::vec3& u) {
  glm::vec3 dir, nc, fc, X, Y, Z;

  // compute the Z axis of camera
  // this axis points in the opposite direction from
  // the looking direction
  Z = glm::normalize(p - l);

  // X axis of camera with given "up" vector and Z axis
  X = glm::normalize(glm::cross(u, Z));

  // the real "up" vector is the cross product of Z and X
  Y = glm::cross(Z, X);

  // compute the centers of the near and far planes
  nc = (p - Z) * nearD;
  fc = (p - Z) * farD;

  /*
  // compute the 4 corners of the frustum on the near plane
  ntl = nc + Y * nh - X * nw;
  ntr = nc + Y * nh + X * nw;
  nbl = nc - Y * nh - X * nw;
  nbr = nc - Y * nh + X * nw;

  // compute the 4 corners of the frustum on the far plane
  ftl = fc + Y * fh - X * fw;
  ftr = fc + Y * fh + X * fw;
  fbl = fc - Y * fh - X * fw;
  fbr = fc - Y * fh + X * fw;

  // compute the six planes
  // the function create_from_three_points assumes that the points
  // are given in counter clockwise order
  planes[(int)FrustumSide::TOP].create_from_three_points(ntr, ntl, ftl);
  planes[(int)FrustumSide::BOTTOM].create_from_three_points(nbl, nbr, fbr);
  planes[(int)FrustumSide::LEFT].create_from_three_points(ntl, nbl, fbl);
  planes[(int)FrustumSide::RIGHT].create_from_three_points(nbr, ntr, fbr);
  planes[(int)FrustumSide::NEAR].create_from_three_points(ntl, ntr, nbr);
  planes[(int)FrustumSide::FAR].create_from_three_points(ftr, ftl, fbl);
  */

  planes[(int)FrustumSide::NEAR] = Plane{.normal = -Z, .point = nc};
  planes[(int)FrustumSide::FAR] = Plane{.normal = Z, .point = fc};

  glm::vec3 aux, normal;

  aux = glm::normalize((nc - Y * nh) - p);
  normal = glm::cross(X, aux);
  planes[(int)FrustumSide::BOTTOM] =
      Plane{.normal = normal, .point = nc - Y * nh};

  aux = glm::normalize((nc + Y * nh) - p);
  normal = glm::cross(aux, X);
  planes[(int)FrustumSide::TOP] = Plane{.normal = normal, .point = nc + Y * nh};

  aux = glm::normalize((nc - X * nw) - p);
  normal = glm::cross(aux, Y);
  planes[(int)FrustumSide::LEFT] =
      Plane{.normal = normal, .point = nc - X * nw};

  aux = glm::normalize((nc + X * nw) - p);
  normal = glm::cross(Y, aux);
  planes[(int)FrustumSide::RIGHT] =
      Plane{.normal = normal, .point = nc + X * nw};
}

// true = inside frustum
bool Frustum::test_point(const glm::vec3& point) {
  PRINT("Point recieved: {} {} {}\n", point.x, point.y, point.z);
  for (const auto& plane : planes) {
    if (plane.signed_distance(point) < 0) {
      return false;
    }
  }
  return true;
}

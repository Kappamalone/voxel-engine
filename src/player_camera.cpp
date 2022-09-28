#include "player_camera.h"
#include "glm/geometric.hpp"

PlayerCamera::PlayerCamera(float fovy, float aspect_ratio, float znear,
                           float zfar)
    : frustum(fovy, aspect_ratio, znear, zfar) {
  this->fovy = fovy;
  this->aspect_ratio = aspect_ratio;
  this->znear = znear;
  this->zfar = zfar;
  projection = glm::perspective(glm::radians(fovy), aspect_ratio, znear, zfar);
}

void PlayerCamera::process_input(Direction direction, float delta_time) {
  if (direction == Direction::FORWARD) {
    camera_pos += camera_front * camera_speed * delta_time;
  }
  if (direction == Direction::BACK) {
    camera_pos -= camera_front * camera_speed * delta_time;
  }
  if (direction == Direction::RIGHT) {
    camera_pos += camera_right * camera_speed * delta_time;
  }
  if (direction == Direction::LEFT) {
    camera_pos -= camera_right * camera_speed * delta_time;
  }
}

// processes input received from a mouse input system. Expects the offset value
// in both the x and y direction.
void PlayerCamera::process_mouse_input(float xoffset, float yoffset) {
  static bool first = true; // on startup
  if (first) {
    first = false;
    return;
  }

  xoffset *= camera_mouse_sensitivity;
  yoffset *= camera_mouse_sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  // make sure that when pitch is out of bounds, screen doesn't get flipped
  if (pitch > 89.0f) {
    pitch = 89.0f;
  }
  if (pitch < -89.0f) {
    pitch = -89.0f;
  }

  calculate_camera_vectors();
}

void PlayerCamera::calculate_camera_vectors() {
  // calculate the new Front vector
  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  camera_front = glm::normalize(front);
  camera_right = glm::normalize(glm::cross(camera_front, world_up));
  camera_up = glm::normalize(glm::cross(camera_right, camera_front));
}

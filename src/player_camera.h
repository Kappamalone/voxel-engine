#pragma once
#include "common.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum class Direction { FORWARD, BACK, LEFT, RIGHT };

class PlayerCamera {
private:
  glm::vec3 camera_pos = glm::vec3(0.0f, 120.0f, 0.0f);
  glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 world_up = camera_up;
  glm::vec3 camera_right = glm::normalize(glm::cross(camera_front, world_up));
  float yaw = -90.0f;
  float pitch = 0.0f;

  glm::mat4 projection;
  glm::mat4 view;

  float camera_speed = 40.f;
  float camera_mouse_sensitivity = 0.1f;

public:
  PlayerCamera(float aspect_ratio);
  void process_input(Direction direction, float delta_time);
  void process_mouse_input(float xoffset, float yoffset);
  void calculate_camera_vectors();

  glm::mat4* get_view_matrix() {
    view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
    return &view;
  }

  glm::mat4* get_projection_matrix() {
    return &projection;
  }

  glm::vec3 get_player_pos() {
    return camera_pos;
  }
};

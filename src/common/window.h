#pragma once
#include "common.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
// glfw3 has to be included after glad
#include <GLFW/glfw3.h>

// TODO: optionally disable imgui setup if not being used?
// abstracts away all non-opengl window tasks
class Window {
private:
  int viewport_width;
  int viewport_height;
  const char* window_name;
  GLFWwindow* window;
  bool is_frame_limited;

  void setup_imgui();

public:
  Window(int viewport_width, int viewport_height, const char* window_name);
  ~Window();

  [[nodiscard]] GLFWwindow* get_window() const {
    return window;
  };

  [[nodiscard]] int get_viewport_width() const {
    return viewport_width;
  };

  [[nodiscard]] int get_viewport_height() const {
    return viewport_height;
  };

  [[nodiscard]] float get_viewport_aspect_ratio() const {
    return (float)viewport_width / (float)viewport_height;
  };

  void toggle_frame_limiting() {
    // FIXME: this is broken due to glfw getting input too fast once
    // framelimiting turned off
    is_frame_limited = !is_frame_limited;
    glfwSwapInterval(is_frame_limited);
  }

  bool key_pressed(int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
  };

  bool key_released(int key) {
    return glfwGetKey(window, key) == GLFW_RELEASE;
  };

  static void imgui_new_frame();
  static void imgui_end_frame();
};

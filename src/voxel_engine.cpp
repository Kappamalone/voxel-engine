#include "voxel_engine.h"

VoxelEngine::VoxelEngine(int viewport_width, int viewport_height)
    : window(viewport_width, viewport_height, "TEMPLATE"),
      player_camera(window.get_viewport_aspect_ratio()),
      chunk_manager(player_camera.get_projection_matrix()) {
  glfwSetInputMode(window.get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void VoxelEngine::run() {
  auto draw_imgui = [&]() {
    bool p_open = true;
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowPos(ImVec2{10.0f, 10.0f}, ImGuiCond_Always);
    ImGui::Begin("FPS", &p_open, window_flags);
    std::string a = fmt::format("Frame time : {:.02f}ms\n", delta_time * 1000.);
    std::string b = fmt::format("FPS        : {:.02f}  \n", 1. / delta_time);
    ImGui::Text(a.c_str());
    ImGui::Text(b.c_str());
    ImGui::Separator();
    ImGui::End();
  };

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(window.get_window())) {
    window.imgui_new_frame();
    handle_input();
    draw_imgui();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.2, 0.3, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    chunk_manager.render_chunks(player_camera.get_player_pos(),
                                *player_camera.get_view_matrix());

    // order is T * R * S to get SRT transformation for model matrix
    // order is P * V * M to get MVP transformation to clip space, then
    // viewport transform to screen space

    window.imgui_end_frame();
    glfwSwapBuffers(window.get_window());
  }
}

void VoxelEngine::handle_input() {
  current_frame = glfwGetTime();
  delta_time = current_frame - last_frame;
  last_frame = current_frame;
  glfwPollEvents();
  if (window.key_pressed(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window.get_window(), true);
  }
  if (window.key_pressed(GLFW_KEY_F)) {
    window.toggle_frame_limiting();
  }
  if (window.key_pressed(GLFW_KEY_P)) {
    toggle_wireframe();
  }
  if (window.key_pressed(GLFW_KEY_W)) {
    player_camera.process_input(Direction::FORWARD, delta_time);
  }
  if (window.key_pressed(GLFW_KEY_S)) {
    player_camera.process_input(Direction::BACK, delta_time);
  }
  if (window.key_pressed(GLFW_KEY_D)) {
    player_camera.process_input(Direction::RIGHT, delta_time);
  }
  if (window.key_pressed(GLFW_KEY_A)) {
    player_camera.process_input(Direction::LEFT, delta_time);
  }

  static double mouse_x = 0;
  static double new_mouse_x = 0;
  static double mouse_y = 0;
  static double new_mouse_y = 0;

  glfwGetCursorPos(window.get_window(), &new_mouse_x, &new_mouse_y);
  player_camera.process_mouse_input(new_mouse_x - mouse_x,
                                    mouse_y - new_mouse_y);
  mouse_x = new_mouse_x;
  mouse_y = new_mouse_y;
}

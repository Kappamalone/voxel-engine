#include "voxel_engine.h"
#include "chunk.h"
#include "player_camera.h"
#include "shader_program.h"

VoxelEngine::VoxelEngine(int viewport_width, int viewport_height)
    : window(viewport_width, viewport_height, "TEMPLATE"), player_camera() {
  glfwSetInputMode(window.get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void VoxelEngine::run() {
  Chunk chunk = Chunk();
  uint32_t vao;
  uint32_t vbo;
  uint32_t ibo;

  glCreateVertexArrays(1, &vao);
  glCreateBuffers(1, &vbo);
  glNamedBufferData(vbo, chunk.vertices_buffer.size() * 4,
                    chunk.vertices_buffer.data(), GL_STATIC_DRAW);
  glCreateBuffers(1, &ibo);
  glNamedBufferData(ibo, chunk.indices_buffer.size() * 4,
                    chunk.indices_buffer.data(), GL_STATIC_DRAW);

  glVertexArrayVertexBuffer(vao, 0, vbo, 0, 4);
  glVertexArrayElementBuffer(vao, ibo);

  glEnableVertexArrayAttrib(vao, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(vao, 0, 0);

  ShaderProgram shader_program =
      ShaderProgram("../src/shaders/voxel.vert", "../src/shaders/voxel.frag",
                    ShaderSourceType::FILE);
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 projection = glm::perspective(
      glm::radians(45.0f), window.get_viewport_aspect_ratio(), 0.1f, 100.0f);
  shader_program.set_uniform_matrix<UniformMSize::FOUR>("model", 1, false,
                                                        glm::value_ptr(model));
  shader_program.set_uniform_matrix<UniformMSize::FOUR>(
      "projection", 1, false, glm::value_ptr(projection));

  while (!glfwWindowShouldClose(window.get_window())) {

    window.imgui_new_frame();

    handle_input();

    // ImGui::ShowDemoWindow();

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

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2, 0.3, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader_program.set_uniform_matrix<UniformMSize::FOUR>(
        "view", 1, false, glm::value_ptr(*player_camera.get_view_matrix()));

    shader_program.use();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindVertexArray(vao);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, chunk.indices_buffer.size(), GL_UNSIGNED_INT,
                   0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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
#include "glm/trigonometric.hpp"
#include "shader_program.h"
#include "window.h"
#include <array>
#include <functional>
#include <iostream>

static constexpr unsigned int SCR_WIDTH = 1400;
static constexpr unsigned int SCR_HEIGHT = 1000;
static constexpr const char* SCR_NAME = "TEMPLATE";
static bool is_frame_limited = true;

// view matrix variables
glm::mat4 view;
glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

float last_x = SCR_WIDTH / 2.0f;
float last_y = SCR_HEIGHT / 2.0f;
float pitch = 0;
float yaw = -90.0f;

static void camera_callback(GLFWwindow* window, double xpos, double ypos) {
  static bool first_mouse = true;
  if (first_mouse) {
    last_x = xpos;
    last_y = ypos;
    first_mouse = false;
  }

  float xoffset = xpos - last_x;
  float yoffset = last_y - ypos;
  last_x = xpos;
  last_y = ypos;

  float sensitivity = 0.1f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  camera_front = glm::normalize(direction);
}

// TODO: opengl core vs compat profile might actually matter
// TODO: bigger font size imgui
int main() {

  Window window = Window(SCR_WIDTH, SCR_HEIGHT, SCR_NAME);

  // vertex data
  // clang-format off
  float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
  };

  
  glm::vec3 cube_positions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f), 
    glm::vec3( 2.0f,  5.0f, -15.0f), 
    glm::vec3(-1.5f, -2.2f, -2.5f),  
    glm::vec3(-3.8f, -2.0f, -12.3f),  
    glm::vec3( 2.4f, -0.4f, -3.5f),  
    glm::vec3(-1.7f,  3.0f, -7.5f),  
    glm::vec3( 1.3f, -2.0f, -2.5f),  
    glm::vec3( 1.5f,  2.0f, -2.5f), 
    glm::vec3( 1.5f,  0.2f, -1.5f), 
    glm::vec3(-1.3f,  1.0f, -1.5f)  
  };

  // clang-format on
  /*
  float vertices[] = {
      // positions        // colors         // texture coords
      0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
      0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
  };
  uint32_t indices[] = {
      // note that we start from 0!
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };
  */
  size_t attributes_per_vertice = 5;

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  /*
  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  */

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                        attributes_per_vertice * sizeof(float), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                        attributes_per_vertice * sizeof(float),
                        (void*)(sizeof(float) * 3));
  glEnableVertexAttribArray(1);

  GLuint tex0;
  glGenTextures(1, &tex0);
  glBindTexture(GL_TEXTURE_2D, tex0);
  // set the texture wrapping/filtering options (on the currently bound texture
  // object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  // texture magnification doesn't use mipmaps
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load and generate the texture
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);
  uint8_t* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    PANIC("Failed to load texture!\n");
  }
  stbi_image_free(data);

  GLuint tex1;
  glGenTextures(1, &tex1);
  glBindTexture(GL_TEXTURE_2D, tex1);
  // set the texture wrapping/filtering options (on the currently bound texture
  // object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load and generate the texture
  stbi_set_flip_vertically_on_load(true);
  data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    PANIC("Failed to load texture!\n");
  }
  stbi_image_free(data);

  // NOTE: i think 0 is a reserved nullptr-esque value
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  glDeleteBuffers(1, &vbo);

  const char* vs_file = "../src/shaders/learnopengl.vert";
  const char* fs_file = "../src/shaders/learnopengl.fragg";
  auto shader_program = ShaderProgram(vs_file, fs_file, ShaderSourceType::FILE);

  // TODO: learn about
  // lambdas, captures and
  // functors

  // texture variables
  float mix_value = 0.2;

  glfwSetInputMode(window.get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window.get_window(), camera_callback);

  // projection matrix variables
  float fov = glm::radians(45.0f);
  float near = 0.1f;
  float far = 100.0f;

  // gui variables
  float input_lock = 0.0f;
  float delta_time = 0.0f;
  float current_frame = 0.0f;
  float last_frame = 0.0f;

  while (!glfwWindowShouldClose(window.get_window())) {
    current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    float camera_speed = 2.5f * delta_time; // adjust accordingly
    window.imgui_new_frame();

    glfwPollEvents();
    if (glfwGetKey(window.get_window(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window.get_window(), true);
    }
    if (window.key_pressed(GLFW_KEY_W)) {
      camera_pos += camera_speed * camera_front;
    }
    if (window.key_pressed(GLFW_KEY_S)) {
      camera_pos -= camera_speed * camera_front;
    }
    if (window.key_pressed(GLFW_KEY_A)) {
      camera_pos -=
          glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
    }
    if (window.key_pressed(GLFW_KEY_D)) {
      camera_pos +=
          glm::normalize(cross(camera_front, camera_up)) * camera_speed;
    }
    if (window.key_pressed(GLFW_KEY_F)) {
      if (glfwGetTime() - input_lock >= 0.1) {
        window.toggle_frame_limiting();
      }
      input_lock = glfwGetTime();
    }
    // camera_pos.y = 0.0f;
    view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);

    ImGui::Begin("Debug");
    ImGui::SliderFloat("fov", &fov, 0, glm::radians(180.0f));
    ImGui::SliderFloat("near", &near, 0.1f, 100.0f);
    ImGui::SliderFloat("far", &far, 10.0f, 400.0f);
    ImGui::SliderFloat("mix value", &mix_value, 0.0f, 1.0f);
    ImGui::End();

    ImGui::ShowDemoWindow();

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

    shader_program.use();
    shader_program.set_uniform("mix_value", mix_value);
    glBindVertexArray(vao);
    glBindTextureUnit(0, tex0);
    glBindTextureUnit(1, tex1);

    // order is T * R * S to get SRT transformation for model matrix
    // order is P * V * M to get MVP transformation to clip space, then
    // viewport transform to screen space

    glm::mat4 projection = glm::mat4(1.0f);
    projection =
        glm::perspective(fov, (float)SCR_WIDTH / (float)SCR_HEIGHT, near, far);

    shader_program.set_uniform_matrix<UniformMSize::FOUR>("view", 1, false,
                                                          glm::value_ptr(view));
    shader_program.set_uniform_matrix<UniformMSize::FOUR>(
        "projection", 1, false, glm::value_ptr(projection));

    for (size_t i = 0; i < 10; i++) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cube_positions[i]);
      model = glm::rotate(model, (float)glfwGetTime() + (float)i * 1.f,
                          glm::vec3(0.5f, 1.0f, 0.0f));
      shader_program.set_uniform_matrix<UniformMSize::FOUR>(
          "model", 1, false, glm::value_ptr(model));

      // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    /*
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(uint32_t),
                   GL_UNSIGNED_INT, 0);
    */

    /*
    // order is T * R * S to get SRT transformation
    trans = glm::mat4(1.0f);
    trans = glm::translate(trans, glm::vec3(-0.5f, .5f, 0.f));
    trans = glm::scale(trans,
                       glm::vec3(1.0f * abs(sinf((float)glfwGetTime())),
                                 1.0f *
    abs(sinf((float)glfwGetTime())), 1.0f));
    shader_program.set_uniform_matrix<UniformMSize::FOUR>(
        "transform", 1, false, glm::value_ptr(trans));

    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(uint32_t),
                   GL_UNSIGNED_INT, 0);
    */

    window.imgui_end_frame();
    glfwSwapBuffers(window.get_window());
  }
}

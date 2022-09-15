#include "window.h"
// TODO: https://www.khronos.org/opengl/wiki/OpenGL_Error
static void message_callback(GLenum source, GLenum type, GLuint id,
                             GLenum severity, GLsizei length,
                             const GLchar* message, const void* userParam) {
  PRINT("GL CALLBACK: {} type = 0x{:X}, severity = 0x{:X}, message = {}\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity,
        message);
}

Window::Window(int viewport_width, int viewport_height,
               const char* window_name) {

  this->viewport_width = viewport_width;
  this->viewport_height = viewport_height;
  this->window_name = window_name;

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  window = glfwCreateWindow(viewport_width, viewport_height, window_name,
                            nullptr, nullptr);

  if (window == nullptr) {
    PRINT("Failed to create GLFW window\n");
    glfwTerminate();
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(
      window, [](GLFWwindow* window, int viewport_width, int viewport_height) {
        glViewport(0, 0, viewport_width, viewport_height);
      });

  is_frame_limited = true;
  glfwSwapInterval(is_frame_limited); // vsync

  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    PANIC("Failed to initialize GLAD");
  }
  glViewport(0, 0, viewport_width, viewport_height);

  // During init, enable debug output
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(message_callback, 0);

  // imgui
  setup_imgui();
}

Window::~Window() {
  glfwTerminate();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Window::setup_imgui() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiDockNodeFlags_NoDockingInCentralNode;
  ImGui::StyleColorsClassic();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 420");
}

void Window::imgui_new_frame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Window::imgui_end_frame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

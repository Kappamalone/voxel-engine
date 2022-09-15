#include "shader_program.h"
#include <fstream>
#include <string>

ShaderProgram::ShaderProgram(const char* vs, const char* fs,
                             ShaderSourceType type) {
  std::string vs_src;
  std::string fs_src;
  if (type == ShaderSourceType::FILE) {
    vs_src = read_file(vs);
    fs_src = read_file(fs);
    vs = vs_src.c_str();
    fs = fs_src.c_str();
  }

  uint32_t vertex_shader;
  uint32_t fragment_shader;
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(vertex_shader, 1, &vs, nullptr);
  glShaderSource(fragment_shader, 1, &fs, nullptr);
  glCompileShader(vertex_shader);
  glCompileShader(fragment_shader);

  ShaderProgram::validate_shader_compilation(vertex_shader);
  ShaderProgram::validate_shader_compilation(fragment_shader);

  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  ShaderProgram::validate_shader_program_linking(shader_program);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
}

void ShaderProgram::validate_shader_compilation(uint32_t shader) {
  static int success;
  static char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    PANIC("Shader compilation failed!\n");
  }
}

void ShaderProgram::validate_shader_program_linking(uint32_t shader_program) {
  static int success;
  static char infoLog[512];
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
  }
}

std::string ShaderProgram::read_file(const char* file_path) {
  std::string content;
  std::ifstream file_stream(file_path, std::ios::in);

  if (!file_stream.is_open()) {
    PANIC("File not found: {}\n", file_path);
  }

  std::string line;
  while (!file_stream.eof()) {
    std::getline(file_stream, line);
    content.append(line + "\n");
  }

  file_stream.close();
  return content;
}

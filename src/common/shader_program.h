#pragma once
#include "common.h"
#include <glad/glad.h>
// glfw3 has to be included after glad
#include <GLFW/glfw3.h>

enum class ShaderSourceType { STRING, FILE };

enum class UniformVSize {
  ONE,
  TWO,
  THREE,
  FOUR,
};

enum class UniformMSize {
  TWO,
  THREE,
  FOUR,
  TWOXTHREE,
  THREEXTWO,
  TWOXFOUR,
  FOURXTWO,
  THREEXFOUR,
  FOURXTHREE
};

// NOTE: .h files are for single line functions and templates, .cpp for the rest

// TODO:  enum class UniformMatrixSize

// TODO: handle other optional shader types like geometry shaders
class ShaderProgram {
private:
  uint32_t shader_program;

public:
  ShaderProgram(const char* vs, const char* fs, ShaderSourceType type);
  ~ShaderProgram() {
    glDeleteProgram(shader_program);
  }

  // TODO: learn about copy/move stuff

  void use() const {
    glUseProgram(shader_program);
  }

  static void unuse() {
    glUseProgram(0);
  }

  // general shader helper functions
  static void validate_shader_compilation(uint32_t shader);
  static void validate_shader_program_linking(uint32_t shader_program);
  static std::string read_file(const char* file_path);

  // TODO: understand how variadic templates actually work... (ha, get it)
  template <typename... T>
  void set_uniform(const char* name, const T&... value) {
    constexpr auto size = sizeof...(T);

    if constexpr (std::disjunction_v<std::is_same<T, float>...>) {
      if constexpr (size == 1) {
        glProgramUniform1f(shader_program,
                           glGetUniformLocation(shader_program, name),
                           value...);
      } else if constexpr (size == 2) {
        glProgramUniform2f(shader_program,
                           glGetUniformLocation(shader_program, name),
                           value...);
      } else if constexpr (size == 3) {
        glProgramUniform3f(shader_program,
                           glGetUniformLocation(shader_program, name),
                           value...);
      } else if constexpr (size == 4) {
        glProgramUniform4f(shader_program,
                           glGetUniformLocation(shader_program, name),
                           value...);
      }
      return;
    }

    if constexpr (std::disjunction_v<std::is_same<T, int>...>) {
      if constexpr (size == 1) {
        glProgramUniform1i(shader_program,
                           glGetUniformLocation(shader_program, name),
                           value...);
      } else if constexpr (size == 2) {
        glProgramUniform2i(shader_program,
                           glGetUniformLocation(shader_program, name),
                           value...);
      } else if constexpr (size == 3) {
        glProgramUniform3i(shader_program,
                           glGetUniformLocation(shader_program, name),
                           value...);
      } else if constexpr (size == 4) {
        glProgramUniform4i(shader_program,
                           glGetUniformLocation(shader_program, name),
                           value...);
      }
      return;
    }

    if constexpr (std::disjunction_v<std::is_same<T, uint32_t>...>) {
      if constexpr (size == 1) {
        glProgramUniform1ui(shader_program,
                            glGetUniformLocation(shader_program, name),
                            value...);
      } else if constexpr (size == 2) {
        glProgramUniform2ui(shader_program,
                            glGetUniformLocation(shader_program, name),
                            value...);
      } else if constexpr (size == 3) {
        glProgramUniform3ui(shader_program,
                            glGetUniformLocation(shader_program, name),
                            value...);
      } else if constexpr (size == 4) {
        glProgramUniform4ui(shader_program,
                            glGetUniformLocation(shader_program, name),
                            value...);
      }
      return;
    }

    PANIC("Invalid shader uniform arguments!\n");
  }

  // NOTE: T can be deduced if placed as the second template parameter?
  template <UniformVSize U, typename T>
  void set_uniform_vector(const char* name, size_t count, T value) {
    if constexpr (std::is_same_v<T, float*>) {
      if constexpr (U == UniformVSize::ONE) {
        glProgramUniform1fv(shader_program,
                            glGetUniformLocation(shader_program, name), count,
                            value);
        // glUniform1fv(glGetUniformLocation(shader_program, name), count,
        // value);
      } else if constexpr (U == UniformVSize::TWO) {
        glProgramUniform2fv(shader_program,
                            glGetUniformLocation(shader_program, name), count,
                            value);
      } else if constexpr (U == UniformVSize::THREE) {
        glProgramUniform3fv(shader_program,
                            glGetUniformLocation(shader_program, name), count,
                            value);
      } else if constexpr (U == UniformVSize::FOUR) {
        glProgramUniform4fv(shader_program,
                            glGetUniformLocation(shader_program, name), count,
                            value);
      }
      return;
    }

    if constexpr (std::is_same_v<T, int*>) {
      if constexpr (U == UniformVSize::ONE) {
        glProgramUniform1iv(shader_program,
                            glGetUniformLocation(shader_program, name), count,
                            value);
      } else if constexpr (U == UniformVSize::TWO) {
        glProgramUniform2iv(shader_program,
                            glGetUniformLocation(shader_program, name), count,
                            value);
      } else if constexpr (U == UniformVSize::THREE) {
        glProgramUniform3iv(shader_program,
                            glGetUniformLocation(shader_program, name), count,
                            value);
      } else if constexpr (U == UniformVSize::FOUR) {
        glProgramUniform4iv(shader_program,
                            glGetUniformLocation(shader_program, name), count,
                            value);
      }
      return;
    }

    if constexpr (std::is_same_v<T, uint32_t*>) {
      if constexpr (U == UniformVSize::ONE) {
        glProgramUniform1uiv(shader_program,
                             glGetUniformLocation(shader_program, name), count,
                             value);
      } else if constexpr (U == UniformVSize::TWO) {
        glProgramUniform2uiv(shader_program,
                             glGetUniformLocation(shader_program, name), count,
                             value);
      } else if constexpr (U == UniformVSize::THREE) {
        glProgramUniform3uiv(shader_program,
                             glGetUniformLocation(shader_program, name), count,
                             value);
      } else if constexpr (U == UniformVSize::FOUR) {
        glProgramUniform4uiv(shader_program,
                             glGetUniformLocation(shader_program, name), count,
                             value);
      }
      return;
    }

    PANIC("Invalid shader uniform arguments!\n");
  }

  template <UniformMSize U, typename T>
  void set_uniform_matrix(const char* name, size_t count, bool transpose,
                          T value) {

    if constexpr (std::is_same_v<T, float*>) {
      if constexpr (U == UniformMSize::TWO) {
        glProgramUniformMatrix2fv(shader_program,
                                  glGetUniformLocation(shader_program, name),
                                  count, transpose, value);
      }
      if constexpr (U == UniformMSize::THREE) {
        glProgramUniformMatrix3fv(shader_program,
                                  glGetUniformLocation(shader_program, name),
                                  count, transpose, value);
      }
      if constexpr (U == UniformMSize::FOUR) {
        glProgramUniformMatrix4fv(shader_program,
                                  glGetUniformLocation(shader_program, name),
                                  count, transpose, value);
      }

      if constexpr (U == UniformMSize::TWOXTHREE) {
        glProgramUniformMatrix2x3fv(shader_program,
                                    glGetUniformLocation(shader_program, name),
                                    count, transpose, value);
      }
      if constexpr (U == UniformMSize::THREEXTWO) {
        glProgramUniformMatrix3x2fv(shader_program,
                                    glGetUniformLocation(shader_program, name),
                                    count, transpose, value);
      }

      if constexpr (U == UniformMSize::TWOXFOUR) {
        glProgramUniformMatrix2x4fv(shader_program,
                                    glGetUniformLocation(shader_program, name),
                                    count, transpose, value);
      }
      if constexpr (U == UniformMSize::FOURXTWO) {
        glProgramUniformMatrix4x2fv(shader_program,
                                    glGetUniformLocation(shader_program, name),
                                    count, transpose, value);
      }

      if constexpr (U == UniformMSize::THREEXFOUR) {
        glProgramUniformMatrix3x4fv(shader_program,
                                    glGetUniformLocation(shader_program, name),
                                    count, transpose, value);
      }
      if constexpr (U == UniformMSize::FOURXTHREE) {
        glProgramUniformMatrix4x3fv(shader_program,
                                    glGetUniformLocation(shader_program, name),
                                    count, transpose, value);
      }

      return;
    }

    PANIC("Invalid shader uniform arguments!\n");
  }
};

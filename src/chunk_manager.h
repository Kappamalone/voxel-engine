#pragma once
#include "chunk.h"

// handles all terrain (chunk) related operations
//
// Chunk manager determines which chunks are in the vicinity of the player
// Then each chunk passes its vertices to be rendered
//
// This means that the shader program and texture atlas ownership should be
// passed to this class instead to reduce overhead from rendering individual
// chunks
class ChunkManager {
private:
  GLuint vao;
  GLuint vbo;
  int cpu_bytes_allocated = 0;
  int gpu_bytes_allocated = 1024 * 1024;
  ShaderProgram shader_program;
  int attributes_per_vertice = 5;

  GLuint tex_atlas;
  int tex_atlas_rows;

  std::vector<Chunk> render_list;

  void manage_chunks();

public:
  ChunkManager(glm::mat4 projection);
  void render_chunks(glm::vec3 pos, glm::mat4 view);
};

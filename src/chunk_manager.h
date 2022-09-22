#pragma once
#include "chunk.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

struct ChunkDrawData {
  glm::vec3 model;
  int offset;
  Chunk* chunk;
};

class ChunkManager {
private:
  GLuint vao;
  GLuint vbo;
  int cpu_bytes_allocated = 0;
  int gpu_bytes_allocated = 1024 * 1024 * 400;
  ShaderProgram shader_program;
  int attributes_per_vertice = 5;

  GLuint tex_atlas;
  int tex_atlas_rows;

  glm::vec3 old_world_pos;
  std::unordered_map<glm::vec3, Chunk> world_chunks;
  std::vector<ChunkDrawData> visible_list;
  std::vector<ChunkDrawData> render_list;

  void manage_chunks(glm::vec3 pos);

  int view_distance = 1;

public:
  ChunkManager(glm::mat4 projection);
  void render_chunks(glm::vec3 pos, glm::mat4 view);
};

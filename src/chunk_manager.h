#pragma once
#include "chunk.h"
#include "frustum.h"
#include "player_camera.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct ChunkDrawData {
  int offset;
  Chunk* chunk;
  ChunkPos chunk_pos;
};

class ChunkManager {
private:
  PlayerCamera& player_camera;

  GLuint vao;
  GLuint vbo;
  int cpu_bytes_allocated = 0;
  int gpu_bytes_allocated = 1024 * 1024 * 400;
  ShaderProgram shader_program;
  int attributes_per_vertice = 5;

  GLuint tex_atlas;
  siv::PerlinNoise perlin_noise;

  ChunkPos old_world_pos;
  std::unordered_map<ChunkPos, Chunk> world_chunks;
  std::vector<ChunkDrawData> visible_list;
  std::vector<ChunkDrawData> render_list;

  void manage_chunks(glm::vec3 pos);

  int view_distance = 10;

  static uint32_t random_seed();

public:
  ChunkManager(PlayerCamera& player_camera);
  void render_chunks();
};

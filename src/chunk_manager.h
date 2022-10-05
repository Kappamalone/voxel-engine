#pragma once
#include "chunk.h"
#include "frustum.h"
#include "player_camera.h"
#include <deque>
#include <thread>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// The chunk draw process:
//  N + 1 chunk data generated around the player (once)
//  N chunk meshes generated around player (once)
//    - separate thread dedicated to generating this data
//  Frustum culling to determine visible meshes (per frame)
//  Render visible meshes (per frame)

struct ChunkDrawData {
  int offset;
  Chunk* chunk;
};

class ChunkManager {
private:
  PlayerCamera& player_camera;

  GLuint vao;
  GLuint vbo;
  int cpu_bytes_allocated = 0;
  int gpu_bytes_allocated = 1024 * 1024 * 100;
  ShaderProgram shader_program;
  int attributes_per_vertice = 5;

  GLuint tex_atlas;
  siv::PerlinNoise perlin_noise;

  ChunkPos old_world_pos;
  std::unordered_map<ChunkPos, Chunk> world_chunks;
  std::vector<ChunkDrawData> visible_list;
  std::vector<ChunkDrawData> render_list;

  std::thread mesh_gen_thread;
  std::deque<Chunk*> mesh_gen_queue;

  void manage_chunks(glm::vec3 pos);

  int view_distance = 5;

  static uint32_t random_seed();

  // used to place structures on top of terrain (trees)
  void place_block(int x, int y, int z, VoxelType voxel_type) {
    ChunkPos world_chunk_pos;
    if (x >= 0) {
      world_chunk_pos.x = (int)(x / CHUNK_WIDTH);
    } else {
      world_chunk_pos.x = floor(x / CHUNK_WIDTH);
    }

    if (z >= 0) {
      world_chunk_pos.z = ceil(z / CHUNK_DEPTH);
    } else {
      world_chunk_pos.z = (int)(z / CHUNK_DEPTH);
    }

    world_chunks.at(world_chunk_pos)
        .set_voxel(x % CHUNK_WIDTH, y, z % CHUNK_DEPTH, voxel_type);
  }

public:
  ChunkManager(PlayerCamera& player_camera);
  void render_chunks();
};

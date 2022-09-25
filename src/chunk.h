#pragma once
#include "PerlinNoise.hpp"
#include "shader_program.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <vector>

static constexpr int CHUNK_WIDTH = 16;
static constexpr int CHUNK_DEPTH = 16;
static constexpr int CHUNK_HEIGHT = 256;
static constexpr float VOXEL_LENGTH = 1.0f;

enum class VoxelType {
  AIR,
  DIRT,
  GRASS,
  STONE,
};
enum class BlockFaces {
  BOTTOM = 0,
  TOP,
  LEFT,
  RIGHT,
  BACK,
  FRONT,
};
enum class TexturePosition {
  BOTTOM_LEFT,
  BOTTOM_RIGHT,
  TOP_LEFT,
  TOP_RIGHT,
};

struct Voxel {
  VoxelType voxel_type;
};

struct ChunkPos {
  int x;
  int z;

  bool operator==(const ChunkPos& other) const {
    return (x == other.x && z == other.z);
  }
};

namespace std {
template <>
struct hash<ChunkPos> {
  size_t operator()(const ChunkPos& c) const {
    return (hash<int>()(c.x)) ^ (hash<int>()(c.z));
  }
};
} // namespace std

// NOTE: uses LH coordinate system for storage of local voxel positions
class Chunk {
private:
  std::unordered_map<ChunkPos, Chunk>& world_chunks;
  siv::PerlinNoise& perlin_noise;

  std::vector<Voxel> voxels;
  std::vector<float> vertices_buffer;
  int xoffset;
  int zoffset;
  bool mesh_created = false;

  void emit_vertex_coordinates(int index, float x, float y, float z);
  void emit_texture_coordinates(TexturePosition position, int atlas_index);
  void construct_face(BlockFaces face, int atlas_index, float x, float y,
                      float z);
  void create_voxels();

  Voxel& get_voxel(int x, int y, int z) {
    return voxels[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH];
  }

  void set_voxel(int x, int y, int z, VoxelType voxel_type) {
    voxels[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH] =
        Voxel{.voxel_type = voxel_type};
  }

  bool is_air_voxel(int x, int y, int z) {
    return get_voxel(x, y, z).voxel_type == VoxelType::AIR;
  }

public:
  Chunk(std::unordered_map<ChunkPos, Chunk>& world_chunks, int xoffset,
        int zoffset, siv::PerlinNoise& perlin_noise);
  void create_mesh();

  float* get_vertices_data() {
    return vertices_buffer.data();
  }

  int get_vertices_byte_size() {
    return vertices_buffer.size() * sizeof(float);
  }

  bool initial_mesh_created() const {
    return mesh_created;
  }

private:
  // clang-format off
  std::unordered_map<VoxelType, std::unordered_map<BlockFaces, int>>
      block_to_faces_map = {
      {VoxelType::GRASS,
      {
        {BlockFaces::BOTTOM, 2},
        {BlockFaces::TOP, 0},
        {BlockFaces::LEFT, 3},
        {BlockFaces::RIGHT, 3},
        {BlockFaces::FRONT, 3},
        {BlockFaces::BACK, 3},
        }
      },
      {VoxelType::STONE,
      {
        {BlockFaces::BOTTOM, 1},
        {BlockFaces::TOP, 1},
        {BlockFaces::LEFT, 1},
        {BlockFaces::RIGHT, 1},
        {BlockFaces::FRONT, 1},
        {BlockFaces::BACK, 1},
        }
      },
      {VoxelType::DIRT,
      {
        {BlockFaces::BOTTOM, 2},
        {BlockFaces::TOP, 2},
        {BlockFaces::LEFT, 2},
        {BlockFaces::RIGHT, 2},
        {BlockFaces::FRONT, 2},
        {BlockFaces::BACK, 2}
      }
  }};
  // clang-format on
};

constexpr auto chunk_vert = R"(
#version 460 core
layout (location = 0) in vec3 vertex_coord;
layout (location = 1) in vec2 _tex_coord;

out vec2 tex_coord;

// uniform mat4 models[64];
// uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  // gl_Position = projection * view * models[gl_DrawID] * vec4(vertex_coord, 1.0);
  // gl_Position = projection * view * model * vec4(vertex_coord, 1.0);
  gl_Position = projection * view * vec4(vertex_coord, 1.0);
  tex_coord = _tex_coord;
}
  )";

constexpr auto chunk_frag = R"(
#version 460 core

layout (binding = 0) uniform sampler2D tex_atlas;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
  frag_color = texture(tex_atlas, tex_coord);
}
  )";

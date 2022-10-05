#pragma once
#include "PerlinNoise.hpp"
#include "shader_program.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <vector>

static constexpr int CHUNK_WIDTH = 32;
static constexpr int CHUNK_DEPTH = 32;
static constexpr int CHUNK_HEIGHT = 256;

enum class VoxelType {
  AIR,
  DIRT,
  GRASS,
  STONE,
  WATER,
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

struct BoundingBox {
  glm::vec3 min;
  glm::vec3 max;
};

// NOTE: uses LH coordinate system for storage of local voxel positions
class Chunk {
private:
  Chunk* f_chunk;
  Chunk* b_chunk;
  Chunk* l_chunk;
  Chunk* r_chunk;
  siv::PerlinNoise& perlin_noise;

  std::vector<Voxel> voxels;
  std::vector<float> vertices_buffer;
  std::vector<float> water_vertices_buffer;
  BoundingBox bounding_box;

  ChunkPos chunk_pos;
  bool mesh_created = false;
  bool mesh_creation_requested = false;

  void emit_vertex_coordinates(int index, float x, float y, float z);
  void emit_texture_coordinates(TexturePosition position, int atlas_index);
  void construct_face(BlockFaces face, int atlas_index, float x, float y,
                      float z);
  void create_voxels();

  Voxel& get_voxel(int x, int y, int z) {
    return voxels[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH];
  }

  bool is_air_voxel(int x, int y, int z) {
    return get_voxel(x, y, z).voxel_type == VoxelType::AIR;
  }

  float get_x_offset() const {
    return chunk_pos.x * CHUNK_WIDTH;
  }

  float get_z_offset() const {
    return chunk_pos.z * CHUNK_DEPTH;
  }

public:
  Chunk(ChunkPos chunk_pos, siv::PerlinNoise& perlin_noise);
  void create_mesh();
  void set_neighbour_chunks(Chunk* u_chunk, Chunk* d_chunk, Chunk* l_chunk,
                            Chunk* r_chunk);

  void set_voxel(int x, int y, int z, VoxelType voxel_type) {
    voxels[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH] =
        Voxel{.voxel_type = voxel_type};
  }

  const float* get_vertices_data() const {
    return vertices_buffer.data();
  }

  const float* get_water_vertices_data() const {
    return water_vertices_buffer.data();
  }

  const BoundingBox& get_bounding_box() const {
    return bounding_box;
  }

  int get_vertices_byte_size() const {
    return vertices_buffer.size() * sizeof(float);
  }

  int get_water_vertices_byte_size() const {
    return water_vertices_buffer.size() * sizeof(float);
  }

  bool initial_mesh_created() const {
    return mesh_created;
  }

  bool has_mesh_requested() const {
    return mesh_creation_requested;
  }

  void request_mesh_creation() {
    mesh_creation_requested = true;
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
      {VoxelType::WATER,
      {
        {BlockFaces::BOTTOM, 192 + 13},
        {BlockFaces::TOP, 192 + 13},
        {BlockFaces::LEFT, 192 + 13},
        {BlockFaces::RIGHT, 192 + 13},
        {BlockFaces::FRONT, 192 + 13},
        {BlockFaces::BACK, 192 + 13},
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

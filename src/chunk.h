#pragma once
#include "shader_program.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <vector>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static constexpr int CHUNK_WIDTH = 16;
static constexpr int CHUNK_DEPTH = 16;
static constexpr int CHUNK_HEIGHT = 16;
static constexpr float VOXEL_LENGTH = 1.0f;

constexpr auto chunk_vert = R"(
#version 460 core
layout (location = 0) in vec3 vertex_coord;
layout (location = 1) in vec2 _tex_coord;

out vec2 tex_coord;

// uniform mat4 models[64];
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * model * vec4(vertex_coord, 1.0);
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

enum class VoxelType {
  AIR,
  DIRT,
  GRASS,
};

struct Voxel {
  VoxelType voxel_type;
};

// a chunk is a 16 * 16 * 256 collection of voxels
// NOTE: uses LH coordinate system for storage of local voxel positions
class Chunk {
private:
  enum class BlockFaces { BOTTOM = 0, TOP, LEFT, RIGHT, BACK, FRONT };
  enum class TexturePosition { BOTTOM_LEFT, BOTTOM_RIGHT, TOP_LEFT, TOP_RIGHT };

  std::vector<Voxel> voxels;
  std::vector<float> vertices_buffer;
  int tex_atlas_rows = 16;

  Voxel& get_voxel(int x, int y, int z) {
    return voxels[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH];
  }

  bool is_air_voxel(int x, int y, int z) {
    return get_voxel(x, y, z).voxel_type == VoxelType::AIR;
  }

  void construct_face(BlockFaces face, int atlas_index, float x, float y,
                      float z);
  // TODO: is there a way to avoid having tex coords sent to the gpu?
  void emit_texture_coordinates(TexturePosition position, int atlas_index);
  void create_mesh();
  void create_voxels();

public:
  Chunk();

  float* get_vertices_data() {
    return vertices_buffer.data();
  }

  int get_vertices_byte_size() {
    return vertices_buffer.size() * sizeof(float);
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

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

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(vertex_coord, 1.0);
    tex_coord = _tex_coord;
}
  )";

constexpr auto chunk_frag = R"(
#version 460 core

layout(binding = 0) uniform sampler2D tex_atlas;

in vec2 tex_coord;
out vec4 frag_color;

void main()
{
    frag_color = texture(tex_atlas, tex_coord);
}
  )";

enum class VoxelType {
  AIR,
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
  int attributes_per_vertice = 5;

  GLuint vao;
  GLuint vbo;
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
  ShaderProgram shader_program;

  GLuint tex_atlas;
  int tex_atlas_rows;

  Voxel& get_voxel(int x, int y, int z) {
    return voxels[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH];
  }

  bool is_air_voxel(int x, int y, int z) {
    return get_voxel(x, y, z).voxel_type == VoxelType::AIR;
  }

  void construct_face(BlockFaces face, int atlas_index, float x, float y,
                      float z);
  void emit_texture_coordinates(TexturePosition position, int atlas_index);
  void create_mesh();
  void create_voxels();

public:
  Chunk(glm::mat4 model, glm::mat4 projection);

  // TODO: any way to group all visible chunks rendering into a single call?
  // that way only model matrix is needed per chunk and is way faster
  void render_mesh(glm::mat4& view);

private:
  std::unordered_map<VoxelType, std::unordered_map<BlockFaces, int>>
      block_to_faces_map = {{VoxelType::GRASS,
                             {
                                 {BlockFaces::BOTTOM, 0},
                                 {BlockFaces::TOP, 5},
                                 {BlockFaces::LEFT, 0},
                                 {BlockFaces::RIGHT, 10},
                                 {BlockFaces::FRONT, 20},
                                 {BlockFaces::BACK, 20},
                             }}

  };
};

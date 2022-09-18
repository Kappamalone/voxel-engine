#pragma once
#include "shader_program.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

// region -> chunk -> block

enum class VoxelType {
  AIR,
  GRASS,
};

struct Voxel {
  VoxelType voxel_type;
};

enum class BlockFaces { BOTTOM = 0, TOP, LEFT, RIGHT, BACK, FRONT };

// a chunk is a 16 * 16 * 256 collection of voxels
// NOTE: uses LH coordinate system for storage of local voxel positions
class Chunk {

private:
  friend class VoxelEngine;
  std::vector<Voxel> voxels;
  std::vector<float> vertices_buffer;

  GLuint vao;
  GLuint vbo;
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
  ShaderProgram shader_program;

  void construct_face(BlockFaces face, float x, float y, float z);
  void create_voxels();
  void create_mesh();

public:
  Chunk(glm::mat4 model, glm::mat4 projection);

  void render_mesh(glm::mat4& view);
};

constexpr auto chunk_vert = R"(
#version 460 core
layout (location = 0) in vec3 vertex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(vertex_coord, 1.0);
}
  )";

constexpr auto chunk_frag = R"(
#version 460 core

out vec4 frag_color;

void main()
{
    frag_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
  )";

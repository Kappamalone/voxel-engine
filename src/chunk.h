#pragma once
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

// a chunk is a 16 * 16 * 256 collection of voxels
class Chunk {

private:
  friend class VoxelEngine;
  std::vector<Voxel> voxels;

  std::vector<float> vertices_buffer;

public:
  Chunk();

  void create_voxels();
  void create_mesh();
};

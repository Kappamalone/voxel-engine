#include "chunk.h"
#include "common.h"

static constexpr int CHUNK_WIDTH = 2;
static constexpr int CHUNK_DEPTH = 8;
static constexpr int CHUNK_HEIGHT = 4;

Chunk::Chunk() {
  create_voxels();
  create_mesh();
}

void Chunk::create_voxels() {
  voxels.resize(1);
  voxels[0] = Voxel{.voxel_type = VoxelType::AIR};
}

void Chunk::create_mesh() {
  static constexpr float VOXEL_SIDE = 1.0f;

  auto x = 0;
  auto y = 0;
  auto z = 0;

  vertices_buffer.push_back(x);
  vertices_buffer.push_back(y);
  vertices_buffer.push_back(-z);

  vertices_buffer.push_back(x);
  vertices_buffer.push_back(y);
  vertices_buffer.push_back(-z - VOXEL_SIDE);

  vertices_buffer.push_back(x + VOXEL_SIDE);
  vertices_buffer.push_back(y);
  vertices_buffer.push_back(-z - VOXEL_SIDE);

  vertices_buffer.push_back(x + VOXEL_SIDE);
  vertices_buffer.push_back(y);
  vertices_buffer.push_back(-z);

  // repeat

  vertices_buffer.push_back(x);
  vertices_buffer.push_back(y + VOXEL_SIDE);
  vertices_buffer.push_back(-z);

  vertices_buffer.push_back(x);
  vertices_buffer.push_back(y + VOXEL_SIDE);
  vertices_buffer.push_back(-z - VOXEL_SIDE);

  vertices_buffer.push_back(x + VOXEL_SIDE);
  vertices_buffer.push_back(y + VOXEL_SIDE);
  vertices_buffer.push_back(-z - VOXEL_SIDE);

  vertices_buffer.push_back(x + VOXEL_SIDE);
  vertices_buffer.push_back(y + VOXEL_SIDE);
  vertices_buffer.push_back(-z);

  // range from (INDEX) -> (INDEX + 11)
  // need 12 indices
  int base_index = 0;

  // bottom
  indices_buffer.push_back(base_index + 0 * 3);
  indices_buffer.push_back(base_index + 1 * 3);
  indices_buffer.push_back(base_index + 3 * 3);

  indices_buffer.push_back(base_index + 1 * 3);
  indices_buffer.push_back(base_index + 2 * 3);
  indices_buffer.push_back(base_index + 3 * 3);

  // top
  indices_buffer.push_back(base_index + 7 * 3);
  indices_buffer.push_back(base_index + 6 * 3);
  indices_buffer.push_back(base_index + 4 * 3);

  indices_buffer.push_back(base_index + 6 * 3);
  indices_buffer.push_back(base_index + 5 * 3);
  indices_buffer.push_back(base_index + 4 * 3);

  // left
  indices_buffer.push_back(base_index + 1 * 3);
  indices_buffer.push_back(base_index + 5 * 3);
  indices_buffer.push_back(base_index + 0 * 3);

  indices_buffer.push_back(base_index + 5 * 3);
  indices_buffer.push_back(base_index + 4 * 3);
  indices_buffer.push_back(base_index + 0 * 3);

  // right
  indices_buffer.push_back(base_index + 3 * 3);
  indices_buffer.push_back(base_index + 7 * 3);
  indices_buffer.push_back(base_index + 2 * 3);

  indices_buffer.push_back(base_index + 7 * 3);
  indices_buffer.push_back(base_index + 6 * 3);
  indices_buffer.push_back(base_index + 2 * 3);

  // front
  indices_buffer.push_back(base_index + 0 * 3);
  indices_buffer.push_back(base_index + 4 * 3);
  indices_buffer.push_back(base_index + 3 * 3);

  indices_buffer.push_back(base_index + 4 * 3);
  indices_buffer.push_back(base_index + 7 * 3);
  indices_buffer.push_back(base_index + 3 * 3);

  // back
  indices_buffer.push_back(base_index + 2 * 3);
  indices_buffer.push_back(base_index + 6 * 3);
  indices_buffer.push_back(base_index + 1 * 3);

  indices_buffer.push_back(base_index + 6 * 3);
  indices_buffer.push_back(base_index + 5 * 3);
  indices_buffer.push_back(base_index + 1 * 3);
}

/*
static constexpr float VOXEL_SIDE = 1.0f;

vertices_buffer.push_back(x);
vertices_buffer.push_back(y);
vertices_buffer.push_back(-z);

vertices_buffer.push_back(x);
vertices_buffer.push_back(y);
vertices_buffer.push_back(-z - VOXEL_SIDE);

vertices_buffer.push_back(x + VOXEL_SIDE);
vertices_buffer.push_back(y);
vertices_buffer.push_back(-z - VOXEL_SIDE);

vertices_buffer.push_back(x + VOXEL_SIDE);
vertices_buffer.push_back(y);
vertices_buffer.push_back(-z);

// repeat

vertices_buffer.push_back(x);
vertices_buffer.push_back(y + VOXEL_SIDE);
vertices_buffer.push_back(-z);

vertices_buffer.push_back(x);
vertices_buffer.push_back(y + VOXEL_SIDE);
vertices_buffer.push_back(-z - VOXEL_SIDE);

vertices_buffer.push_back(x + VOXEL_SIDE);
vertices_buffer.push_back(y + VOXEL_SIDE);
vertices_buffer.push_back(-z - VOXEL_SIDE);

vertices_buffer.push_back(x + VOXEL_SIDE);
vertices_buffer.push_back(y + VOXEL_SIDE);
vertices_buffer.push_back(-z);

// range from (INDEX) -> (INDEX + 11)
// need 12 indices
int base_index = (x + z * 16 + y * CHUNK_HEIGHT * 16) * 8;

// bottom
indices_buffer.push_back(base_index);
indices_buffer.push_back(base_index + 1);
indices_buffer.push_back(base_index + 3);

indices_buffer.push_back(base_index + 1);
indices_buffer.push_back(base_index + 2);
indices_buffer.push_back(base_index + 3);

// top
indices_buffer.push_back(base_index + 4);
indices_buffer.push_back(base_index + 5);
indices_buffer.push_back(base_index + 7);

indices_buffer.push_back(base_index + 5);
indices_buffer.push_back(base_index + 6);
indices_buffer.push_back(base_index + 7);

// left
indices_buffer.push_back(base_index);
indices_buffer.push_back(base_index + 4);
indices_buffer.push_back(base_index + 1);

indices_buffer.push_back(base_index + 4);
indices_buffer.push_back(base_index + 5);
indices_buffer.push_back(base_index + 1);

// right
indices_buffer.push_back(base_index + 3);
indices_buffer.push_back(base_index + 7);
indices_buffer.push_back(base_index + 2);

indices_buffer.push_back(base_index + 7);
indices_buffer.push_back(base_index + 6);
indices_buffer.push_back(base_index + 2);

// front
indices_buffer.push_back(base_index);
indices_buffer.push_back(base_index + 4);
indices_buffer.push_back(base_index + 3);

indices_buffer.push_back(base_index + 4);
indices_buffer.push_back(base_index + 7);
indices_buffer.push_back(base_index + 3);

// back
indices_buffer.push_back(base_index + 1);
indices_buffer.push_back(base_index + 5);
indices_buffer.push_back(base_index + 2);

indices_buffer.push_back(base_index + 5);
indices_buffer.push_back(base_index + 6);
indices_buffer.push_back(base_index + 2);
*/

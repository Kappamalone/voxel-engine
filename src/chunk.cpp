#include "chunk.h"
#include "common.h"
#include <algorithm>

static constexpr int CHUNK_WIDTH = 16;
static constexpr int CHUNK_DEPTH = 16;
static constexpr int CHUNK_HEIGHT = 16;
static constexpr float VOXEL_SIDE = 1.0f;

bool random_bool() {
  return rand() > (RAND_MAX / 2);
}

Chunk::Chunk() {
  create_voxels();
  create_mesh();
}

void Chunk::create_voxels() {
  voxels.resize(CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT);
  for (auto y = 0; y < CHUNK_HEIGHT; y++) {
    for (auto z = 0; z < CHUNK_DEPTH; z++) {
      for (auto x = 0; x < CHUNK_WIDTH; x++) {
        if (random_bool()) {
          voxels[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH] =
              Voxel{.voxel_type = VoxelType::GRASS};
        } else {
          voxels[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH] =
              Voxel{.voxel_type = VoxelType::AIR};
        }
      }
    }
  }
}

void Chunk::construct_face(BlockFaces face, float x, float y, float z) {
  using fp =
      void(std::vector<float> & vertices_buffer, float x, float y, float z);
  static fp* vertices[] = {
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x);
        vertices_buffer.push_back(y);
        vertices_buffer.push_back(-z);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x);
        vertices_buffer.push_back(y);
        vertices_buffer.push_back(-z - VOXEL_SIDE);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x + VOXEL_SIDE);
        vertices_buffer.push_back(y);
        vertices_buffer.push_back(-z - VOXEL_SIDE);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x + VOXEL_SIDE);
        vertices_buffer.push_back(y);
        vertices_buffer.push_back(-z);
      },
      // repeat
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x);
        vertices_buffer.push_back(y + VOXEL_SIDE);
        vertices_buffer.push_back(-z);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x);
        vertices_buffer.push_back(y + VOXEL_SIDE);
        vertices_buffer.push_back(-z - VOXEL_SIDE);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x + VOXEL_SIDE);
        vertices_buffer.push_back(y + VOXEL_SIDE);
        vertices_buffer.push_back(-z - VOXEL_SIDE);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x + VOXEL_SIDE);
        vertices_buffer.push_back(y + VOXEL_SIDE);
        vertices_buffer.push_back(-z);
      },
  };

  switch ((BlockFaces)face) {
    case BlockFaces::BOTTOM:
      vertices[2](vertices_buffer, x, y, z);
      vertices[3](vertices_buffer, x, y, z);
      vertices[0](vertices_buffer, x, y, z);

      vertices[0](vertices_buffer, x, y, z);
      vertices[1](vertices_buffer, x, y, z);
      vertices[2](vertices_buffer, x, y, z);
      break;
    case BlockFaces::TOP:
      vertices[7](vertices_buffer, x, y, z);
      vertices[6](vertices_buffer, x, y, z);
      vertices[5](vertices_buffer, x, y, z);

      vertices[5](vertices_buffer, x, y, z);
      vertices[4](vertices_buffer, x, y, z);
      vertices[7](vertices_buffer, x, y, z);
      break;
    case BlockFaces::LEFT:
      vertices[0](vertices_buffer, x, y, z);
      vertices[4](vertices_buffer, x, y, z);
      vertices[5](vertices_buffer, x, y, z);

      vertices[5](vertices_buffer, x, y, z);
      vertices[1](vertices_buffer, x, y, z);
      vertices[0](vertices_buffer, x, y, z);
      break;
    case BlockFaces::RIGHT:
      vertices[2](vertices_buffer, x, y, z);
      vertices[6](vertices_buffer, x, y, z);
      vertices[7](vertices_buffer, x, y, z);

      vertices[7](vertices_buffer, x, y, z);
      vertices[3](vertices_buffer, x, y, z);
      vertices[2](vertices_buffer, x, y, z);
      break;
    case BlockFaces::FRONT:
      vertices[3](vertices_buffer, x, y, z);
      vertices[7](vertices_buffer, x, y, z);
      vertices[4](vertices_buffer, x, y, z);

      vertices[4](vertices_buffer, x, y, z);
      vertices[0](vertices_buffer, x, y, z);
      vertices[3](vertices_buffer, x, y, z);
      break;
    case BlockFaces::BACK:
      vertices[1](vertices_buffer, x, y, z);
      vertices[5](vertices_buffer, x, y, z);
      vertices[6](vertices_buffer, x, y, z);

      vertices[6](vertices_buffer, x, y, z);
      vertices[2](vertices_buffer, x, y, z);
      vertices[1](vertices_buffer, x, y, z);
      break;
  }
}

void Chunk::create_mesh() {

  // algorithm:
  //  for each voxel that isn't an air type, check if any of it's six faces
  //  borders an air block, if so add that face to the mesh, else ignore
  //
  //  TODO: texturing and texture atlases
  //  TODO: make size of blocks actually variable
  //  TODO: handle chunk border vertices culling

  for (auto y = 0; y < CHUNK_HEIGHT; y++) {
    for (auto z = 0; z < CHUNK_DEPTH; z++) {
      for (auto x = 0; x < CHUNK_WIDTH; x++) {
        // check if current voxel isn't an air block
        auto index = x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH;
        if (voxels[index].voxel_type == VoxelType::AIR) {
          continue;
        }

        for (auto face = (int)BlockFaces::BOTTOM; face < 6; face++) {
          switch ((BlockFaces)face) {
            case BlockFaces::BOTTOM: {
              if (y == 0) {
                construct_face(BlockFaces::BOTTOM, x, y, z);
                continue;
              }
              if (voxels[x + z * CHUNK_WIDTH +
                         (y - 1) * CHUNK_WIDTH * CHUNK_DEPTH]
                      .voxel_type == VoxelType::AIR) {
                construct_face(BlockFaces::BOTTOM, x, y, z);
              }
              break;
            }
            case BlockFaces::TOP: {
              if (y == CHUNK_HEIGHT - 1) {
                construct_face(BlockFaces::TOP, x, y, z);
                continue;
              }
              if (voxels[x + z * CHUNK_WIDTH +
                         (y + 1) * CHUNK_WIDTH * CHUNK_DEPTH]
                      .voxel_type == VoxelType::AIR) {
                construct_face(BlockFaces::TOP, x, y, z);
              }
              break;
            }
            case BlockFaces::LEFT: {
              if (x == 0) {
                construct_face(BlockFaces::LEFT, x, y, z);
                continue;
              }
              if (voxels[(x - 1) + z * CHUNK_WIDTH +
                         y * CHUNK_WIDTH * CHUNK_DEPTH]
                      .voxel_type == VoxelType::AIR) {
                construct_face(BlockFaces::LEFT, x, y, z);
              }
              break;
            }
            case BlockFaces::RIGHT: {
              if (x == CHUNK_WIDTH - 1) {
                construct_face(BlockFaces::RIGHT, x, y, z);
                continue;
              }
              if (voxels[(x + 1) + z * CHUNK_WIDTH +
                         y * CHUNK_WIDTH * CHUNK_DEPTH]
                      .voxel_type == VoxelType::AIR) {
                construct_face(BlockFaces::RIGHT, x, y, z);
              }
              break;
            }
            case BlockFaces::FRONT: {
              if (z == 0) {
                construct_face(BlockFaces::FRONT, x, y, z);
                continue;
              }
              if (voxels[x + (z - 1) * CHUNK_WIDTH +
                         y * CHUNK_WIDTH * CHUNK_DEPTH]
                      .voxel_type == VoxelType::AIR) {
                construct_face(BlockFaces::FRONT, x, y, z);
              }
              break;
            }
            case BlockFaces::BACK: {
              if (z == CHUNK_DEPTH - 1) {
                construct_face(BlockFaces::BACK, x, y, z);
                continue;
              }
              if (voxels[x + (z + 1) * CHUNK_WIDTH +
                         y * CHUNK_WIDTH * CHUNK_DEPTH]
                      .voxel_type == VoxelType::AIR) {
                construct_face(BlockFaces::BACK, x, y, z);
              }
              break;
            }
          }
        }
      }
    }
  }
}

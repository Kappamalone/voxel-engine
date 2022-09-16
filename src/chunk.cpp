#include "chunk.h"
#include "common.h"
#include <algorithm>

static constexpr int CHUNK_WIDTH = 16;
static constexpr int CHUNK_DEPTH = 16;
static constexpr int CHUNK_HEIGHT = 256;
static constexpr float VOXEL_SIDE = 1.0f;

bool random_bool() {
  return rand() > (RAND_MAX / 2);
}

enum class BlockFaces { BOTTOM = 0, TOP, LEFT, RIGHT, BACK, FRONT };

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

void Chunk::create_mesh() {

  // algorithm:
  //  for each voxel that isn't an air type, check if any of it's six faces
  //  borders an air block, if so add that face to the mesh, else ignore

  using fp =
      void(std::vector<float> & vertices_buffer, float x, float y, float z);
  fp* vertices[] = {
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

  // TODO: learn about and fix winding order to be clockwise
  // TODO: face culling?
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
              auto dy = std::max(y - 1, 0);
              if (voxels[x + z * CHUNK_WIDTH + dy * CHUNK_WIDTH * CHUNK_DEPTH]
                          .voxel_type == VoxelType::AIR ||
                  y == 0) {
                vertices[0](vertices_buffer, x, y, z);
                vertices[1](vertices_buffer, x, y, z);
                vertices[3](vertices_buffer, x, y, z);

                vertices[1](vertices_buffer, x, y, z);
                vertices[2](vertices_buffer, x, y, z);
                vertices[3](vertices_buffer, x, y, z);
                break;
              }
            }
            case BlockFaces::TOP: {
              auto dy = std::min(y + 1, CHUNK_HEIGHT);
              if (voxels[x + z * CHUNK_WIDTH + dy * CHUNK_WIDTH * CHUNK_DEPTH]
                          .voxel_type == VoxelType::AIR ||
                  y == CHUNK_HEIGHT - 1) {
                vertices[4](vertices_buffer, x, y, z);
                vertices[6](vertices_buffer, x, y, z);
                vertices[7](vertices_buffer, x, y, z);

                vertices[4](vertices_buffer, x, y, z);
                vertices[5](vertices_buffer, x, y, z);
                vertices[6](vertices_buffer, x, y, z);
                break;
              }
            }
            case BlockFaces::LEFT: {
              auto dx = std::max(x - 1, 0);
              if (voxels[dx + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH]
                          .voxel_type == VoxelType::AIR ||
                  x == 0) {
                vertices[1](vertices_buffer, x, y, z);
                vertices[5](vertices_buffer, x, y, z);
                vertices[0](vertices_buffer, x, y, z);

                vertices[5](vertices_buffer, x, y, z);
                vertices[4](vertices_buffer, x, y, z);
                vertices[0](vertices_buffer, x, y, z);
                break;
              }
            }
            case BlockFaces::RIGHT: {
              auto dx = std::min(x + 1, CHUNK_WIDTH);
              if (voxels[dx + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH]
                          .voxel_type == VoxelType::AIR ||
                  x == CHUNK_WIDTH - 1) {
                vertices[3](vertices_buffer, x, y, z);
                vertices[7](vertices_buffer, x, y, z);
                vertices[2](vertices_buffer, x, y, z);

                vertices[7](vertices_buffer, x, y, z);
                vertices[6](vertices_buffer, x, y, z);
                vertices[2](vertices_buffer, x, y, z);
              }
              break;
            }
            case BlockFaces::BACK: {
              auto dz = std::min(z + 1, CHUNK_DEPTH);
              if (voxels[x + dz * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH]
                          .voxel_type == VoxelType::AIR ||
                  z == CHUNK_DEPTH - 1) {
                vertices[2](vertices_buffer, x, y, z);
                vertices[6](vertices_buffer, x, y, z);
                vertices[1](vertices_buffer, x, y, z);

                vertices[6](vertices_buffer, x, y, z);
                vertices[5](vertices_buffer, x, y, z);
                vertices[1](vertices_buffer, x, y, z);
                break;
              }
            }
            case BlockFaces::FRONT: {
              auto dz = std::max(z - 1, 0);
              if (voxels[x + dz * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH]
                          .voxel_type == VoxelType::AIR ||
                  z == 0) {
                vertices[0](vertices_buffer, x, y, z);
                vertices[3](vertices_buffer, x, y, z);
                vertices[4](vertices_buffer, x, y, z);

                vertices[4](vertices_buffer, x, y, z);
                vertices[7](vertices_buffer, x, y, z);
                vertices[3](vertices_buffer, x, y, z);
                break;
              }
            }
          }
        }
      }
    }
  }
}

#include "chunk.h"
#include "common.h"
#include <algorithm>

static constexpr int CHUNK_WIDTH = 16;
static constexpr int CHUNK_DEPTH = 16;
static constexpr int CHUNK_HEIGHT = 16;
static constexpr float VOXEL_LENGTH = 1.0f;

Chunk::Chunk(glm::mat4 model, glm::mat4 projection)
    : model(model), projection(projection),
      shader_program(chunk_vert, chunk_frag, ShaderSourceType::STRING) {
  create_voxels();
  create_mesh();

  // FIXME: GL_STATIC_DRAW?
  glCreateVertexArrays(1, &vao);
  glCreateBuffers(1, &vbo);
  glNamedBufferData(vbo, vertices_buffer.size() * 4, vertices_buffer.data(),
                    GL_STATIC_DRAW);

  glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(float) * 3);

  glEnableVertexArrayAttrib(vao, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(vao, 0, 0);

  shader_program.set_uniform_matrix<UniformMSize::FOUR>("model", 1, false,
                                                        glm::value_ptr(model));
  shader_program.set_uniform_matrix<UniformMSize::FOUR>(
      "projection", 1, false, glm::value_ptr(projection));
}

void Chunk::render_mesh(glm::mat4& view) {
  static int attributes_per_vertice = 3;
  shader_program.set_uniform_matrix<UniformMSize::FOUR>("view", 1, false,
                                                        glm::value_ptr(view));

  shader_program.use();
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0,
               vertices_buffer.size() / attributes_per_vertice);
}

void Chunk::create_voxels() {
  voxels.resize(CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT);
  for (auto y = 0; y < CHUNK_HEIGHT; y++) {
    for (auto z = 0; z < CHUNK_DEPTH; z++) {
      for (auto x = 0; x < CHUNK_WIDTH; x++) {
        voxels[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH] =
            Voxel{.voxel_type = VoxelType::GRASS};
      }
    }
  }
}

void Chunk::construct_face(BlockFaces face, float x, float y, float z) {
  using fp =
      void(std::vector<float> & vertices_buffer, float x, float y, float z);
  static fp* vertices[] = {
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH);
        vertices_buffer.push_back(-z * VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH);
        vertices_buffer.push_back((-z) * VOXEL_LENGTH - VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH);
        vertices_buffer.push_back((-z) * VOXEL_LENGTH - VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH);
        vertices_buffer.push_back((-z) * VOXEL_LENGTH);
      },
      // repeat
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(-z * VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back((-z) * VOXEL_LENGTH - VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back((-z) * VOXEL_LENGTH - VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(-z * VOXEL_LENGTH);
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
